#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "fish.h"

#include "gl_loader.h"
#include "shader.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct FishVertex {
    float px, py, pz;
    float nx, ny, nz;
};

struct LineVertex {
    float px, py, pz;
};

// Clownfish OBJ swims along +Z; carp OBJ along +X (head at +X). PTF uses local +X as forward.
static const Quat kClownfishAxisFix = quatFromAxisAngle({0.0f, 1.0f, 0.0f}, 3.14159265f * 0.5f);
// Carp mesh nose points -X in file; flip 180 deg around Y so it swims forward along PTF tangent.
static const Quat kCarpAxisFix = quatFromAxisAngle({0.0f, 1.0f, 0.0f}, 3.14159265f);
static constexpr float kFishTargetLength = 1.05f;

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

static std::string resolveClownfishModelPath() {
    const char *names[] = {
        "assets/models/clownfish.obj",
        "assets/models/Clownfish_Low_Poly.obj",
        "assets/models/clownfish_low_poly.obj",
    };
    for (const char *name : names) {
        const std::string path = resolveAssetPath(name);
        if (fileExists(path)) return path;
    }
    return resolveAssetPath(names[0]);
}

static std::string resolveCarpModelPath() {
    const char *names[] = {
        "assets/models/carp.obj",
        "assets/models/carp_with_armature.obj",
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

static bool loadObjMesh(const std::string &path, std::vector<FishVertex> &out, Vec3 &outCenter, float &outMaxExtent) {
    std::ifstream file(path);
    if (!file) return false;

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    positions.reserve(1024);
    normals.reserve(1024);

    Vec3 minB{1e9f, 1e9f, 1e9f};
    Vec3 maxB{-1e9f, -1e9f, -1e9f};

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
            minB.x = std::min(minB.x, p.x);
            minB.y = std::min(minB.y, p.y);
            minB.z = std::min(minB.z, p.z);
            maxB.x = std::max(maxB.x, p.x);
            maxB.y = std::max(maxB.y, p.y);
            maxB.z = std::max(maxB.z, p.z);
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
                    out.push_back({p.x, p.y, p.z, n.x, n.y, n.z});
                }
            }
        }
    }

    if (out.empty()) return false;

    outCenter = {
        (minB.x + maxB.x) * 0.5f,
        (minB.y + maxB.y) * 0.5f,
        (minB.z + maxB.z) * 0.5f,
    };
    const float extX = maxB.x - minB.x;
    const float extY = maxB.y - minB.y;
    const float extZ = maxB.z - minB.z;
    outMaxExtent = std::max(extX, std::max(extY, extZ));
    if (outMaxExtent < 1e-5f) outMaxExtent = 1.0f;
    return true;
}

static void uploadFishMesh(FishMeshGPU &meshGpu, const std::vector<FishVertex> &mesh) {
    glGenVertexArrays_(1, &meshGpu.vao);
    glGenBuffers_(1, &meshGpu.vbo);
    glBindVertexArray_(meshGpu.vao);
    glBindBuffer_(kGL_ARRAY_BUFFER, meshGpu.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.size() * sizeof(FishVertex)), mesh.data(), kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(FishVertex), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray_(1);
    glVertexAttribPointer_(1, 3, kGL_FLOAT, 0, sizeof(FishVertex), reinterpret_cast<void *>(offsetof(FishVertex, nx)));
    glBindVertexArray_(0);
    meshGpu.count = static_cast<GLuint>(mesh.size());
    meshGpu.loaded = meshGpu.count > 0;
}

static bool loadFishMeshAsset(const std::string &path, FishMeshGPU &meshGpu, Quat axisFix) {
    std::vector<FishVertex> mesh;
    float maxExtent = 1.0f;
    if (!loadObjMesh(path, mesh, meshGpu.meshCenter, maxExtent)) {
        return false;
    }
    meshGpu.meshScale = kFishTargetLength / maxExtent;
    meshGpu.meshAxisFix = axisFix;
    uploadFishMesh(meshGpu, mesh);
    return meshGpu.loaded;
}

static Mat4 fishModelMatrix(const FishMeshGPU &meshGpu, Vec3 pos, const ParallelTransportFrame &frame) {
    const Mat4 center = mat4Translation(vec3Scale(meshGpu.meshCenter, -1.0f));
    const Mat4 fix = mat4FromQuat(meshGpu.meshAxisFix);
    const Mat4 body = mat4ModelPTF(pos, frame, meshGpu.meshScale);
    return mat4Multiply(body, mat4Multiply(fix, center));
}

static void rebuildPathLines(FishGPU &fish) {
    std::vector<LineVertex> pathLines;
    std::vector<LineVertex> ptfLines;
    constexpr int kPathSamples = 96;
    constexpr int kAxisSamples = 12;
    constexpr float kAxisLen = 0.55f;

    for (const FishInstance &inst : fish.fish) {
        Vec3 prev = splinePosition(inst.path, 0.0f);
        for (int i = 1; i <= kPathSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(kPathSamples);
            const Vec3 cur = splinePosition(inst.path, t);
            pathLines.push_back({prev.x, prev.y, prev.z});
            pathLines.push_back({cur.x, cur.y, cur.z});
            prev = cur;
        }

        ParallelTransportFrame frame{};
        initParallelTransportFrame(frame, splineTangent(inst.path, 0.0f), {0.0f, 1.0f, 0.0f});
        for (int i = 0; i <= kAxisSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(kAxisSamples);
            const Vec3 pos = splinePosition(inst.path, t);
            const Vec3 tangent = splineTangent(inst.path, t);
            transportParallelFrame(frame, tangent);

            const Vec3 tEnd = vec3Add(pos, vec3Scale(frame.tangent, kAxisLen));
            const Vec3 nEnd = vec3Add(pos, vec3Scale(frame.normal, kAxisLen * 0.85f));
            const Vec3 bEnd = vec3Add(pos, vec3Scale(frame.binormal, kAxisLen * 0.85f));
            ptfLines.push_back({pos.x, pos.y, pos.z});
            ptfLines.push_back({tEnd.x, tEnd.y, tEnd.z});
            ptfLines.push_back({pos.x, pos.y, pos.z});
            ptfLines.push_back({nEnd.x, nEnd.y, nEnd.z});
            ptfLines.push_back({pos.x, pos.y, pos.z});
            ptfLines.push_back({bEnd.x, bEnd.y, bEnd.z});
        }
    }

    std::vector<LineVertex> lines;
    lines.insert(lines.end(), pathLines.begin(), pathLines.end());
    lines.insert(lines.end(), ptfLines.begin(), ptfLines.end());

    if (!fish.pathVao) glGenVertexArrays_(1, &fish.pathVao);
    if (!fish.pathVbo) glGenBuffers_(1, &fish.pathVbo);
    glBindVertexArray_(fish.pathVao);
    glBindBuffer_(kGL_ARRAY_BUFFER, fish.pathVbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(lines.size() * sizeof(LineVertex)), lines.data(), kGL_DYNAMIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(LineVertex), reinterpret_cast<void *>(0));
    glBindVertexArray_(0);
    fish.pathLineCount = static_cast<GLuint>(pathLines.size());
    fish.ptfLineCount = static_cast<GLuint>(ptfLines.size());
}

static FishInstance makeFish(const Vec3 *points, int count, float speed, FishMeshKind meshKind, float r, float g, float b, float startT) {
    FishInstance inst{};
    buildSplinePath(inst.path, points, count);
    inst.speed = speed;
    inst.t = startT;
    inst.meshKind = meshKind;
    inst.baseColor[0] = r;
    inst.baseColor[1] = g;
    inst.baseColor[2] = b;
    return inst;
}

static FishMeshGPU &fishMeshFor(FishGPU &fish, FishMeshKind kind) {
    return fish.meshes[static_cast<int>(kind)];
}

static const FishMeshGPU &fishMeshFor(const FishGPU &fish, FishMeshKind kind) {
    return fish.meshes[static_cast<int>(kind)];
}

void initFish(FishGPU &fish, float /*waterLevel*/) {
    fish.program = createProgramFromFiles("shaders/fish.vert", "shaders/fish.frag");
    fish.uViewProj = glGetUniformLocation_(fish.program, "uViewProj");
    fish.uModel = glGetUniformLocation_(fish.program, "uModel");
    fish.uWaterLevel = glGetUniformLocation_(fish.program, "uWaterLevel");
    fish.uFogDensity = glGetUniformLocation_(fish.program, "uFogDensity");
    fish.uCameraPos = glGetUniformLocation_(fish.program, "uCameraPos");
    fish.uLightDir = glGetUniformLocation_(fish.program, "uLightDir");
    fish.uLightColor = glGetUniformLocation_(fish.program, "uLightColor");
    fish.uSpotPos = glGetUniformLocation_(fish.program, "uSpotPos");
    fish.uSpotDir = glGetUniformLocation_(fish.program, "uSpotDir");
    fish.uSpotColor = glGetUniformLocation_(fish.program, "uSpotColor");
    fish.uSpotInner = glGetUniformLocation_(fish.program, "uSpotInner");
    fish.uSpotOuter = glGetUniformLocation_(fish.program, "uSpotOuter");
    fish.uSpotIntensity = glGetUniformLocation_(fish.program, "uSpotIntensity");
    fish.uDeepColor = glGetUniformLocation_(fish.program, "uDeepColor");
    fish.uBaseColor = glGetUniformLocation_(fish.program, "uBaseColor");
    fish.uExposure = glGetUniformLocation_(fish.program, "uExposure");

    fish.pathProgram = createProgramFromFiles("shaders/path.vert", "shaders/path.frag");
    fish.uPathViewProj = glGetUniformLocation_(fish.pathProgram, "uViewProj");
    fish.uPathColor = glGetUniformLocation_(fish.pathProgram, "uColor");

    const bool clownfishOk = loadFishMeshAsset(resolveClownfishModelPath(), fishMeshFor(fish, FishMeshKind::Clownfish), kClownfishAxisFix);
    const bool carpOk = loadFishMeshAsset(resolveCarpModelPath(), fishMeshFor(fish, FishMeshKind::Carp), kCarpAxisFix);
    fish.loaded = clownfishOk || carpOk;
    if (!fish.loaded) {
        MessageBoxA(
            nullptr,
            "Missing fish models.\nCopy clownfish.obj and carp.obj to assets\\models\\ and rebuild.",
            "Fish models skipped",
            MB_OK | MB_ICONWARNING
        );
        return;
    }

    static const Vec3 kPathA[] = {
        {-14.0f, -9.0f, 6.0f},
        {-4.0f, -7.5f, 12.0f},
        {8.0f, -8.5f, 4.0f},
        {14.0f, -10.0f, -6.0f},
        {6.0f, -9.0f, -14.0f},
        {-10.0f, -8.0f, -10.0f},
    };
    static const Vec3 kPathB[] = {
        {18.0f, -12.0f, -2.0f},
        {10.0f, -11.0f, 10.0f},
        {-2.0f, -13.0f, 14.0f},
        {-16.0f, -12.5f, 6.0f},
        {-18.0f, -11.5f, -8.0f},
        {-6.0f, -12.0f, -16.0f},
        {8.0f, -13.0f, -12.0f},
    };
    static const Vec3 kPathC[] = {
        {0.0f, -6.5f, 0.0f},
        {6.0f, -7.0f, 5.0f},
        {10.0f, -8.0f, -2.0f},
        {4.0f, -7.5f, -8.0f},
        {-6.0f, -6.8f, -6.0f},
        {-8.0f, -7.2f, 4.0f},
    };
    static const Vec3 kPathD[] = {
        {-10.0f, -9.5f, -8.0f},
        {2.0f, -8.0f, -14.0f},
        {14.0f, -9.0f, -8.0f},
        {16.0f, -10.0f, 4.0f},
        {4.0f, -9.5f, 12.0f},
        {-12.0f, -10.0f, 6.0f},
    };
    static const Vec3 kPathE[] = {
        {-18.0f, -11.0f, 2.0f},
        {-8.0f, -10.0f, 14.0f},
        {6.0f, -11.5f, 16.0f},
        {16.0f, -12.0f, 4.0f},
        {12.0f, -11.0f, -10.0f},
        {-4.0f, -10.5f, -14.0f},
    };

    struct FishSpawnSpec {
        const Vec3 *points;
        int pointCount;
        float speed;
        FishMeshKind kind;
        float r;
        float g;
        float b;
        float startT;
    };

    static const FishSpawnSpec kSpawns[] = {
        {kPathA, static_cast<int>(sizeof(kPathA) / sizeof(kPathA[0])), 2.6f, FishMeshKind::Clownfish, 0.98f, 0.48f, 0.10f, 0.00f},
        {kPathA, static_cast<int>(sizeof(kPathA) / sizeof(kPathA[0])), 2.9f, FishMeshKind::Clownfish, 1.00f, 0.55f, 0.18f, 0.42f},
        {kPathB, static_cast<int>(sizeof(kPathB) / sizeof(kPathB[0])), 1.9f, FishMeshKind::Carp, 0.68f, 0.50f, 0.22f, 0.00f},
        {kPathB, static_cast<int>(sizeof(kPathB) / sizeof(kPathB[0])), 2.1f, FishMeshKind::Carp, 0.62f, 0.46f, 0.20f, 0.38f},
        {kPathC, static_cast<int>(sizeof(kPathC) / sizeof(kPathC[0])), 3.2f, FishMeshKind::Clownfish, 0.95f, 0.42f, 0.12f, 0.15f},
        {kPathC, static_cast<int>(sizeof(kPathC) / sizeof(kPathC[0])), 3.0f, FishMeshKind::Clownfish, 0.92f, 0.38f, 0.08f, 0.58f},
        {kPathD, static_cast<int>(sizeof(kPathD) / sizeof(kPathD[0])), 2.4f, FishMeshKind::Carp, 0.70f, 0.52f, 0.24f, 0.25f},
        {kPathD, static_cast<int>(sizeof(kPathD) / sizeof(kPathD[0])), 2.7f, FishMeshKind::Carp, 0.66f, 0.48f, 0.21f, 0.67f},
        {kPathE, static_cast<int>(sizeof(kPathE) / sizeof(kPathE[0])), 2.2f, FishMeshKind::Clownfish, 0.97f, 0.50f, 0.14f, 0.33f},
        {kPathE, static_cast<int>(sizeof(kPathE) / sizeof(kPathE[0])), 2.5f, FishMeshKind::Carp, 0.64f, 0.47f, 0.19f, 0.71f},
    };

    fish.fish.clear();
    fish.fish.reserve(sizeof(kSpawns) / sizeof(kSpawns[0]));
    for (const FishSpawnSpec &spec : kSpawns) {
        FishMeshKind kind = spec.kind;
        if (kind == FishMeshKind::Clownfish && !clownfishOk) continue;
        if (kind == FishMeshKind::Carp && !carpOk) {
            if (!clownfishOk) continue;
            kind = FishMeshKind::Clownfish;
        }
        fish.fish.push_back(makeFish(spec.points, spec.pointCount, spec.speed, kind, spec.r, spec.g, spec.b, spec.startT));
    }

    rebuildPathLines(fish);
}

void updateFish(FishGPU &fish, float dt) {
    const bool paused = fish.displayMode == FishDisplayMode::PausedWithPath;
    for (FishInstance &inst : fish.fish) {
        if (!paused && inst.path.totalLength > 0.01f) {
            inst.t += (inst.speed / inst.path.totalLength) * dt;
            inst.t -= std::floor(inst.t);
        }

        const Vec3 tangent = splineTangent(inst.path, inst.t);
        if (!inst.frameReady) {
            initParallelTransportFrame(inst.frame, tangent, {0.0f, 1.0f, 0.0f});
            inst.frameReady = true;
        } else {
            transportParallelFrame(inst.frame, tangent);
        }
    }
}

void cycleFishDisplayMode(FishGPU &fish) {
    switch (fish.displayMode) {
    case FishDisplayMode::Normal:
        fish.displayMode = FishDisplayMode::ShowPath;
        break;
    case FishDisplayMode::ShowPath:
        fish.displayMode = FishDisplayMode::PausedWithPath;
        break;
    default:
        fish.displayMode = FishDisplayMode::Normal;
        break;
    }
}

const char *fishDisplayModeLabel(FishDisplayMode mode) {
    switch (mode) {
    case FishDisplayMode::Normal: return "fish move";
    case FishDisplayMode::ShowPath: return "path+PTF";
    default: return "paused+path";
    }
}

static void drawPathLines(const FishGPU &fish, const Mat4 &viewProj) {
    if (fish.pathLineCount == 0) return;

    glUseProgram_(fish.pathProgram);
    glUniformMatrix4fv_(fish.uPathViewProj, 1, 0, viewProj.m);
    glBindVertexArray_(fish.pathVao);
    glLineWidth(2.0f);

    glUniform3f_(fish.uPathColor, 0.35f, 0.85f, 1.0f);
    glDrawArrays(kGL_LINES, 0, static_cast<GLsizei>(fish.pathLineCount));

    if (fish.ptfLineCount > 0) {
        const GLsizei ptfStart = static_cast<GLsizei>(fish.pathLineCount);
        const GLsizei ptfPerFish = 13 * 6;
        for (size_t i = 0; i < fish.fish.size(); ++i) {
            const GLsizei base = ptfStart + static_cast<GLsizei>(i * ptfPerFish);
            glUniform3f_(fish.uPathColor, 1.0f, 0.35f, 0.30f);
            glDrawArrays(kGL_LINES, base, ptfPerFish / 3);
            glUniform3f_(fish.uPathColor, 0.35f, 1.0f, 0.45f);
            glDrawArrays(kGL_LINES, base + ptfPerFish / 3, ptfPerFish / 3);
            glUniform3f_(fish.uPathColor, 0.45f, 0.55f, 1.0f);
            glDrawArrays(kGL_LINES, base + (2 * ptfPerFish) / 3, ptfPerFish / 3);
        }
    }

    glBindVertexArray_(0);
    glUseProgram_(0);
}

void drawFish(const FishGPU &fish, const Mat4 &viewProj, float /*time*/, Vec3 cameraPos, float waterLevel, float fogDensity, Vec3 moonDir, Vec3 moonColor, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity, float exposure, Vec3 deepColor) {
    if (!fish.loaded) return;

    if (fish.displayMode != FishDisplayMode::Normal) {
        drawPathLines(fish, viewProj);
    }

    glEnable(kGL_CULL_FACE);
    glUseProgram_(fish.program);
    glUniformMatrix4fv_(fish.uViewProj, 1, 0, viewProj.m);
    glUniform1f_(fish.uWaterLevel, waterLevel);
    glUniform1f_(fish.uFogDensity, fogDensity);
    glUniform3f_(fish.uCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform3f_(fish.uLightDir, moonDir.x, moonDir.y, moonDir.z);
    glUniform3f_(fish.uLightColor, moonColor.x, moonColor.y, moonColor.z);
    glUniform3f_(fish.uSpotPos, spotPos.x, spotPos.y, spotPos.z);
    glUniform3f_(fish.uSpotDir, spotDir.x, spotDir.y, spotDir.z);
    glUniform3f_(fish.uSpotColor, spotColor.x, spotColor.y, spotColor.z);
    glUniform1f_(fish.uSpotInner, spotInner);
    glUniform1f_(fish.uSpotOuter, spotOuter);
    glUniform1f_(fish.uSpotIntensity, spotIntensity);
    glUniform3f_(fish.uDeepColor, deepColor.x, deepColor.y, deepColor.z);
    glUniform1f_(fish.uExposure, exposure);

    glBindVertexArray_(0);
    for (const FishInstance &inst : fish.fish) {
        const FishMeshGPU &meshGpu = fishMeshFor(fish, inst.meshKind);
        if (!meshGpu.loaded) continue;

        const Vec3 pos = splinePosition(inst.path, inst.t);
        const Mat4 model = fishModelMatrix(meshGpu, pos, inst.frame);
        glBindVertexArray_(meshGpu.vao);
        glUniformMatrix4fv_(fish.uModel, 1, 0, model.m);
        glUniform3f_(fish.uBaseColor, inst.baseColor[0], inst.baseColor[1], inst.baseColor[2]);
        glDrawArrays(kGL_TRIANGLES, 0, static_cast<GLsizei>(meshGpu.count));
    }
    glBindVertexArray_(0);
    glUseProgram_(0);
}

void destroyFish(FishGPU &fish) {
    for (int i = 0; i < static_cast<int>(FishMeshKind::Count); ++i) {
        FishMeshGPU &meshGpu = fish.meshes[i];
        if (meshGpu.vbo) glDeleteBuffers_(1, &meshGpu.vbo);
        if (meshGpu.vao) glDeleteVertexArrays_(1, &meshGpu.vao);
    }
    if (fish.program) glDeleteProgram_(fish.program);
    if (fish.pathVbo) glDeleteBuffers_(1, &fish.pathVbo);
    if (fish.pathVao) glDeleteVertexArrays_(1, &fish.pathVao);
    if (fish.pathProgram) glDeleteProgram_(fish.pathProgram);
    fish = {};
}
