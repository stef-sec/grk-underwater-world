#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "submarine.h"

#include "gl_loader.h"
#include "lighting.h"
#include "math.h"
#include "shader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct SubmarineVertex {
    float px, py, pz;
    float nx, ny, nz;
};

static bool fileExists(const std::string &path) {
    FILE *f = nullptr;
    if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) return false;
    fclose(f);
    return true;
}

static uint64_t fileSizeBytes(const std::string &path) {
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &data)) return 0;
    ULARGE_INTEGER size{};
    size.HighPart = data.nFileSizeHigh;
    size.LowPart = data.nFileSizeLow;
    return size.QuadPart;
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

static std::string resolveSubmarineModelPath() {
    const char *names[] = {
        "assets/models/submarine.obj",
        "assets/models/submarine2.obj",
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

static bool loadObjMesh(const std::string &path, std::vector<SubmarineVertex> &out, float &minY, float &maxY) {
    std::ifstream file(path);
    if (!file) return false;

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    positions.reserve(65536);
    normals.reserve(65536);
    minY = 1e9f;
    maxY = -1e9f;

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
                const int ids[3] = {vi[0], vi[i], vi[i + 1]};
                const int nids[3] = {ni[0], ni[i], ni[i + 1]};
                Vec3 triPos[3]{};
                bool valid = true;
                for (int k = 0; k < 3; ++k) {
                    if (ids[k] < 0 || ids[k] >= static_cast<int>(positions.size())) {
                        valid = false;
                        break;
                    }
                    triPos[k] = positions[ids[k]];
                }
                if (!valid) continue;
                const Vec3 faceNormal = vec3Normalize(vec3Cross(vec3Subtract(triPos[1], triPos[0]), vec3Subtract(triPos[2], triPos[0])));
                for (int k = 0; k < 3; ++k) {
                    Vec3 n = faceNormal;
                    if (nids[k] >= 0 && nids[k] < static_cast<int>(normals.size())) {
                        n = normals[nids[k]];
                    }
                    const Vec3 p = positions[ids[k]];
                    out.push_back(SubmarineVertex{p.x, p.y, p.z, n.x, n.y, n.z});
                }
            }
        }
    }
    return !out.empty();
}

static void uploadMesh(SubmarineGPU &submarine, const std::vector<SubmarineVertex> &vertices) {
    submarine.count = static_cast<GLuint>(vertices.size());
    glGenVertexArrays_(1, &submarine.vao);
    glBindVertexArray_(submarine.vao);
    glGenBuffers_(1, &submarine.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, submarine.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(SubmarineVertex)), vertices.data(), kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(SubmarineVertex), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray_(1);
    glVertexAttribPointer_(1, 3, kGL_FLOAT, 0, sizeof(SubmarineVertex), reinterpret_cast<void *>(offsetof(SubmarineVertex, nx)));
    glBindVertexArray_(0);
}

void initSubmarine(SubmarineGPU &submarine) {
    submarine.program = createProgramFromFiles("shaders/submarine.vert", "shaders/submarine.frag");
    submarine.uViewProj = glGetUniformLocation_(submarine.program, "uViewProj");
    submarine.uModel = glGetUniformLocation_(submarine.program, "uModel");
    submarine.uWaterLevel = glGetUniformLocation_(submarine.program, "uWaterLevel");
    submarine.uFogDensity = glGetUniformLocation_(submarine.program, "uFogDensity");
    submarine.uCameraPos = glGetUniformLocation_(submarine.program, "uCameraPos");
    submarine.uLightDir = glGetUniformLocation_(submarine.program, "uLightDir");
    submarine.uLightColor = glGetUniformLocation_(submarine.program, "uLightColor");
    submarine.uSpotPos = glGetUniformLocation_(submarine.program, "uSpotPos");
    submarine.uSpotDir = glGetUniformLocation_(submarine.program, "uSpotDir");
    submarine.uSpotColor = glGetUniformLocation_(submarine.program, "uSpotColor");
    submarine.uSpotInner = glGetUniformLocation_(submarine.program, "uSpotInner");
    submarine.uSpotOuter = glGetUniformLocation_(submarine.program, "uSpotOuter");
    submarine.uSpotIntensity = glGetUniformLocation_(submarine.program, "uSpotIntensity");
    submarine.uDeepColor = glGetUniformLocation_(submarine.program, "uDeepColor");
    submarine.uBaseColor = glGetUniformLocation_(submarine.program, "uBaseColor");
    submarine.uExposure = glGetUniformLocation_(submarine.program, "uExposure");

    const std::string path = resolveSubmarineModelPath();
    constexpr uint64_t kMaxObjBytes = 64ull * 1024ull * 1024ull;
    const uint64_t objSize = fileSizeBytes(path);
    if (objSize > kMaxObjBytes) {
        MessageBoxA(
            nullptr,
            "submarine.obj is too large for the built-in OBJ loader.\n"
            "Export a lighter low-poly OBJ (recommended under 64 MB) and try again.",
            "Submarine model skipped",
            MB_OK | MB_ICONWARNING
        );
        return;
    }

    std::vector<SubmarineVertex> vertices;
    float minY = 0.0f;
    float maxY = 0.0f;
    if (!loadObjMesh(path, vertices, minY, maxY)) {
        return;
    }

    submarine.minY = minY;
    submarine.maxY = maxY;
    uploadMesh(submarine, vertices);
    submarine.loaded = true;
}

void drawSubmarine(const SubmarineGPU &submarine, const Mat4 &viewProj, const Mat4 &model, Vec3 cameraPos, float waterLevel, float fogDensity, Vec3 moonDir, Vec3 moonColor, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity, float exposure) {
    if (!submarine.loaded || submarine.count == 0) return;

    glEnable(kGL_CULL_FACE);
    glCullFace(kGL_BACK);
    glUseProgram_(submarine.program);
    glUniformMatrix4fv_(submarine.uViewProj, 1, 0, viewProj.m);
    glUniformMatrix4fv_(submarine.uModel, 1, 0, model.m);
    glUniform1f_(submarine.uWaterLevel, waterLevel);
    glUniform1f_(submarine.uFogDensity, fogDensity);
    glUniform3f_(submarine.uCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform3f_(submarine.uLightDir, moonDir.x, moonDir.y, moonDir.z);
    glUniform3f_(submarine.uLightColor, moonColor.x, moonColor.y, moonColor.z);
    glUniform3f_(submarine.uSpotPos, spotPos.x, spotPos.y, spotPos.z);
    glUniform3f_(submarine.uSpotDir, spotDir.x, spotDir.y, spotDir.z);
    glUniform3f_(submarine.uSpotColor, spotColor.x, spotColor.y, spotColor.z);
    glUniform1f_(submarine.uSpotInner, spotInner);
    glUniform1f_(submarine.uSpotOuter, spotOuter);
    glUniform1f_(submarine.uSpotIntensity, spotIntensity);
    glUniform3f_(submarine.uDeepColor, kNightFogColor.x, kNightFogColor.y, kNightFogColor.z);
    glUniform3f_(submarine.uBaseColor, 0.46f, 0.48f, 0.54f);
    glUniform1f_(submarine.uExposure, exposure);
    glBindVertexArray_(submarine.vao);
    glDrawArrays(kGL_TRIANGLES, 0, static_cast<GLsizei>(submarine.count));
    glBindVertexArray_(0);
    glUseProgram_(0);
    glDisable(kGL_CULL_FACE);
}

void destroySubmarine(SubmarineGPU &submarine) {
    if (submarine.program) glDeleteProgram_(submarine.program);
    if (submarine.vbo) glDeleteBuffers_(1, &submarine.vbo);
    if (submarine.vao) glDeleteVertexArrays_(1, &submarine.vao);
    submarine = {};
}
