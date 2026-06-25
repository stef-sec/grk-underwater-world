#pragma once

#include "gl_loader.h"
#include "math.h"
#include "spline.h"

enum class FishDisplayMode {
    Normal = 0,
    ShowPath = 1,
    PausedWithPath = 2,
};

enum class FishMeshKind {
    Clownfish = 0,
    Carp = 1,
    Count = 2,
};

struct FishInstance {
    SplinePath path{};
    float t = 0.0f;
    float speed = 2.5f;
    ParallelTransportFrame frame{};
    bool frameReady = false;
    FishMeshKind meshKind = FishMeshKind::Clownfish;
    float baseColor[3]{0.9f, 0.5f, 0.2f};
};

struct FishMeshGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint count = 0;
    bool loaded = false;
    Vec3 meshCenter{};
    float meshScale = 1.0f;
    Quat meshAxisFix{};
};

struct FishGPU {
    FishMeshGPU meshes[static_cast<int>(FishMeshKind::Count)]{};
    bool loaded = false;

    GLuint pathVao = 0;
    GLuint pathVbo = 0;
    GLuint pathProgram = 0;
    GLuint pathLineCount = 0;
    GLuint ptfLineCount = 0;

    std::vector<FishInstance> fish;
    FishDisplayMode displayMode = FishDisplayMode::Normal;

    GLuint program = 0;
    GLint uViewProj = -1;
    GLint uModel = -1;
    GLint uWaterLevel = -1;
    GLint uFogDensity = -1;
    GLint uCameraPos = -1;
    GLint uLightDir = -1;
    GLint uLightColor = -1;
    GLint uSpotPos = -1;
    GLint uSpotDir = -1;
    GLint uSpotColor = -1;
    GLint uSpotInner = -1;
    GLint uSpotOuter = -1;
    GLint uSpotIntensity = -1;
    GLint uDeepColor = -1;
    GLint uBaseColor = -1;
    GLint uExposure = -1;

    GLint uPathViewProj = -1;
    GLint uPathColor = -1;
};

void initFish(FishGPU &fish, float waterLevel);
void updateFish(FishGPU &fish, float dt);
void cycleFishDisplayMode(FishGPU &fish);
const char *fishDisplayModeLabel(FishDisplayMode mode);
void drawFish(const FishGPU &fish, const Mat4 &viewProj, float time, Vec3 cameraPos, float waterLevel, float fogDensity, Vec3 moonDir, Vec3 moonColor, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity, float exposure, Vec3 deepColor);
void destroyFish(FishGPU &fish);
