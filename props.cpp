#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "props.h"
#include "gl_loader.h"
#include "lighting.h"
#include "shader.h"
#include "terrain.h"

#include <GL/gl.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct PropVertex {
    float px, py, pz;
    float nx, ny, nz;
};

static float hash2i(int x, int y) {
    uint32_t n = static_cast<uint32_t>(x * 374761393u + y * 668265263u);
    n = (n ^ (n >> 13u)) * 1274126177u;
    return static_cast<float>((n ^ (n >> 16u)) & 0x00ffffffu) / 16777215.0f;
}

static bool fileExists(const std::string &path) {
    FILE *f = nullptr;
    if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) return false;
    fclose(f);
    return true;
}

static std::string getExeDir() {
    char path[MAX_PATH]{};
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string s(path);
    const size_t pos = s.find_last_of("\\/");
    if (pos != std::string::npos) s.resize(pos);
    return s;
}

static std::string resolveAssetPath(const char *relative) {
    const std::string rel(relative);
    const std::string exeDir = getExeDir();
    const std::string candidates[] = {
        exeDir + "\\" + rel,
        exeDir + "\\..\\" + rel,
        exeDir + "\\..\\..\\" + rel,
        rel,
    };
    for (const auto &candidate : candidates) {
        if (fileExists(candidate)) return candidate;
    }
    return exeDir + "\\" + rel;
}

static std::string resolveSeaweedModelPath() {
    const char *names[] = {
        "assets/models/seaweed.obj",
        "assets/models/seaweedList.obj",
    };
    for (const char *name : names) {
        const std::string path = resolveAssetPath(name);
        if (fileExists(path)) return path;
    }
    return resolveAssetPath(names[0]);
}

static int parseObjIndex(const std::string &token, int count) {
    if (token.empty()) return -1;
    const size_t slash = token.find('/');
    const std::string idxStr = slash == std::string::npos ? token : token.substr(0, slash);
    int idx = std::stoi(idxStr);
    if (idx < 0) return count + idx;
    return idx - 1;
}

static bool loadObjMesh(const std::string &path, std::vector<PropVertex> &out, float &minY, float &maxY, float &minX, float &maxX, float &minZ, float &maxZ) {
    std::ifstream file(path);
    if (!file) return false;

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    positions.reserve(8192);
    normals.reserve(8192);
    minY = minX = minZ = 1e9f;
    maxY = maxX = maxZ = -1e9f;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;
        if (tag == "v") {
            Vec3 p{};
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minZ = std::min(minZ, p.z);
            maxZ = std::max(maxZ, p.z);
        } else if (tag == "vn") {
            Vec3 n{};
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        } else if (tag == "f") {
            std::vector<int> vi;
            std::vector<int> ni;
            std::string token;
            while (ss >> token) {
                const size_t slash1 = token.find('/');
                const size_t slash2 = token.find('/', slash1 == std::string::npos ? 0 : slash1 + 1);
                vi.push_back(parseObjIndex(token.substr(0, slash1), static_cast<int>(positions.size())));
                if (slash2 != std::string::npos && slash2 + 1 < token.size()) {
                    ni.push_back(parseObjIndex(token.substr(slash2 + 1), static_cast<int>(normals.size())));
                } else {
                    ni.push_back(-1);
                }
            }
            if (vi.size() < 3) continue;
            for (size_t i = 1; i + 1 < vi.size(); ++i) {
                const int ids[3] = {vi[0], vi[static_cast<size_t>(i)], vi[static_cast<size_t>(i + 1)]};
                const int nids[3] = {ni[0], ni[static_cast<size_t>(i)], ni[static_cast<size_t>(i + 1)]};
                Vec3 triPos[3]{};
                bool valid = true;
                for (int k = 0; k < 3; ++k) {
                    if (ids[k] < 0 || ids[k] >= static_cast<int>(positions.size())) {
                        valid = false;
                        break;
                    }
                    triPos[k] = positions[static_cast<size_t>(ids[k])];
                }
                if (!valid) continue;
                const Vec3 faceNormal = vec3Normalize(vec3Cross(vec3Subtract(triPos[1], triPos[0]), vec3Subtract(triPos[2], triPos[0])));
                for (int k = 0; k < 3; ++k) {
                    if (ids[k] < 0 || ids[k] >= static_cast<int>(positions.size())) continue;
                    Vec3 n = faceNormal;
                    if (nids[k] >= 0 && nids[k] < static_cast<int>(normals.size())) {
                        n = normals[static_cast<size_t>(nids[k])];
                    }
                    const Vec3 p = positions[static_cast<size_t>(ids[k])];
                    out.push_back(PropVertex{p.x, p.y, p.z, n.x, n.y, n.z});
                }
            }
        }
    }
    return !out.empty();
}

static void uploadMesh(SeaweedGPU &seaweed, const std::vector<PropVertex> &vertices) {
    seaweed.count = static_cast<GLuint>(vertices.size());
    glGenVertexArrays_(1, &seaweed.vao);
    glBindVertexArray_(seaweed.vao);
    glGenBuffers_(1, &seaweed.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, seaweed.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(PropVertex)), vertices.data(), kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(PropVertex), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray_(1);
    glVertexAttribPointer_(1, 3, kGL_FLOAT, 0, sizeof(PropVertex), reinterpret_cast<void *>(offsetof(PropVertex, nx)));
    glBindVertexArray_(0);
}

static void scatterInstances(SeaweedGPU &seaweed, float waterLevel) {
    seaweed.instances.clear();
    constexpr int kTargetCount = 60;
    int placed = 0;
    for (int i = 0; placed < kTargetCount && i < 400; ++i) {
        const float hx = hash2i(i, i * 3 + 1);
        const float hz = hash2i(i * 5 + 2, i + 7);
        const float x = (hx - 0.5f) * kTerrainWidth * 0.88f;
        const float z = (hz - 0.5f) * kTerrainDepth * 0.88f;
        const float ground = terrainHeight(x, z, 0.0f);
        if (ground > waterLevel - 2.5f) continue;

        const float scale = 1.15f + hash2i(i + 13, i + 17) * 0.45f;
        const float rot = hash2i(i + 19, i + 23) * 6.2831853f;
        const float y = ground - seaweed.minY * scale;
        seaweed.instances.push_back(SeaweedInstance{x, y, z, scale, rot, false});
        ++placed;
    }
}

void initSeaweed(SeaweedGPU &seaweed, float waterLevel) {
    seaweed.program = createProgramFromFiles("shaders/seaweed.vert", "shaders/seaweed.frag");
    seaweed.uViewProj = glGetUniformLocation_(seaweed.program, "uViewProj");
    seaweed.uModel = glGetUniformLocation_(seaweed.program, "uModel");
    seaweed.uTime = glGetUniformLocation_(seaweed.program, "uTime");
    seaweed.uWaterLevel = glGetUniformLocation_(seaweed.program, "uWaterLevel");
    seaweed.uFogDensity = glGetUniformLocation_(seaweed.program, "uFogDensity");
    seaweed.uLightDir = glGetUniformLocation_(seaweed.program, "uLightDir");
    seaweed.uDeepColor = glGetUniformLocation_(seaweed.program, "uDeepColor");
    seaweed.uColor = glGetUniformLocation_(seaweed.program, "uColor");
    seaweed.uSpotPos = glGetUniformLocation_(seaweed.program, "uSpotPos");
    seaweed.uSpotDir = glGetUniformLocation_(seaweed.program, "uSpotDir");
    seaweed.uSpotColor = glGetUniformLocation_(seaweed.program, "uSpotColor");
    seaweed.uSpotInner = glGetUniformLocation_(seaweed.program, "uSpotInner");
    seaweed.uSpotOuter = glGetUniformLocation_(seaweed.program, "uSpotOuter");
    seaweed.uSpotIntensity = glGetUniformLocation_(seaweed.program, "uSpotIntensity");
    seaweed.uExposure = glGetUniformLocation_(seaweed.program, "uExposure");

    const std::string path = resolveSeaweedModelPath();
    std::vector<PropVertex> vertices;
    float minY = 0.0f;
    float maxY = 1.0f;
    float minX = 0.0f;
    float maxX = 0.0f;
    float minZ = 0.0f;
    float maxZ = 0.0f;
    if (!loadObjMesh(path, vertices, minY, maxY, minX, maxX, minZ, maxZ)) {
        return;
    }

    seaweed.minY = minY;
    seaweed.spanX = std::max(0.01f, maxX - minX);
    seaweed.spanZ = std::max(0.01f, maxZ - minZ);
    uploadMesh(seaweed, vertices);
    scatterInstances(seaweed, waterLevel);
    seaweed.loaded = true;
}

void drawSeaweed(const SeaweedGPU &seaweed, const Mat4 &viewProj, float time, float waterLevel, float fogDensity, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity, float exposure, int highlightedIndex) {
    if (!seaweed.loaded || seaweed.instances.empty()) return;

    glDisable(kGL_CULL_FACE);
    glUseProgram_(seaweed.program);
    glUniformMatrix4fv_(seaweed.uViewProj, 1, 0, viewProj.m);
    glUniform1f_(seaweed.uTime, time);
    glUniform1f_(seaweed.uWaterLevel, waterLevel);
    glUniform1f_(seaweed.uFogDensity, fogDensity);
    glUniform3f_(seaweed.uLightDir, kMoonLightDir.x, kMoonLightDir.y, kMoonLightDir.z);
    glUniform3f_(seaweed.uDeepColor, kNightFogColor.x, kNightFogColor.y, kNightFogColor.z);
    glUniform3f_(seaweed.uSpotPos, spotPos.x, spotPos.y, spotPos.z);
    glUniform3f_(seaweed.uSpotDir, spotDir.x, spotDir.y, spotDir.z);
    glUniform3f_(seaweed.uSpotColor, spotColor.x, spotColor.y, spotColor.z);
    glUniform1f_(seaweed.uSpotInner, spotInner);
    glUniform1f_(seaweed.uSpotOuter, spotOuter);
    glUniform1f_(seaweed.uSpotIntensity, spotIntensity);
    glUniform1f_(seaweed.uExposure, exposure);
    glBindVertexArray_(seaweed.vao);
    for (size_t i = 0; i < seaweed.instances.size(); ++i) {
        const SeaweedInstance &inst = seaweed.instances[i];
        const Mat4 model = mat4Model({inst.x, inst.y, inst.z}, inst.rotY, inst.scale);
        glUniformMatrix4fv_(seaweed.uModel, 1, 0, model.m);
        const float tint = 0.85f + hash2i(static_cast<int>(inst.x * 17.0f), static_cast<int>(inst.z * 23.0f)) * 0.15f;
        if (inst.collected) {
            glUniform3f_(seaweed.uColor, 0.04f * tint, 0.18f * tint, 0.30f * tint);
        } else if (static_cast<int>(i) == highlightedIndex) {
            glUniform3f_(seaweed.uColor, 0.42f * tint, 0.78f * tint, 0.20f * tint);
        } else {
            glUniform3f_(seaweed.uColor, 0.08f * tint, 0.42f * tint, 0.18f * tint);
        }
        glDrawArrays(kGL_TRIANGLES, 0, static_cast<GLsizei>(seaweed.count));
    }
    glBindVertexArray_(0);
    glUseProgram_(0);
}

void destroySeaweed(SeaweedGPU &seaweed) {
    if (seaweed.program) glDeleteProgram_(seaweed.program);
    if (seaweed.vbo) glDeleteBuffers_(1, &seaweed.vbo);
    if (seaweed.vao) glDeleteVertexArrays_(1, &seaweed.vao);
    seaweed = {};
}

