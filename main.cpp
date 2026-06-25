#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <vector>

#include "camera.h"
#include "fish.h"
#include "gl_loader.h"
#include "hud.h"
#include "lighting.h"
#include "math.h"
#include "particles.h"
#include "props.h"
#include "shader.h"
#include "shadow.h"
#include "skybox.h"
#include "submarine.h"
#include "terrain.h"
#include "volumetric.h"
#include "water.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

static constexpr int kWidth = 1280;
static constexpr int kHeight = 720;
static constexpr float kCameraClearance = 0.6f;
static constexpr float kMoveSpeed = 9.0f;
static constexpr float kVerticalSpeed = 6.0f;
static constexpr float kTurnSpeed = 1.6f;
static constexpr float kSurfaceClearance = -1.1f;
static constexpr float kSubmarineScale = 0.018f;

static HDC g_hdc = nullptr;
static HGLRC g_hrc = nullptr;
static HWND g_hwnd = nullptr;
static bool g_running = true;

static TerrainGPU g_terrain;
static WaterGPU g_water;
static SkyboxGPU g_skybox;
static ShadowGPU g_shadow;
static SeaweedGPU g_seaweed;
static RockGPU g_rocks;
static SubmarineGPU g_submarine;
static FishGPU g_fish;
static VolumetricGPU g_volumetric;
static ParticlesGPU g_particles;
static HudGPU g_hud;

static GLuint g_terrainProgram = 0;
static GLuint g_waterProgram = 0;
static GLuint g_sandNormalMap = 0;
static GLuint g_rockNormalMap = 0;

static GLint g_uTerrainViewProj = -1;
static GLint g_uTerrainLightVP = -1;
static GLint g_uTerrainTime = -1;
static GLint g_uTerrainWaterLevel = -1;
static GLint g_uTerrainFogDensity = -1;
static GLint g_uTerrainCameraPos = -1;
static GLint g_uTerrainDeepColor = -1;
static GLint g_uTerrainLightDir = -1;
static GLint g_uTerrainLightColor = -1;
static GLint g_uTerrainAmbient = -1;
static GLint g_uTerrainShadowMap = -1;
static GLint g_uTerrainSandBaseColor = -1;
static GLint g_uTerrainSandMetallic = -1;
static GLint g_uTerrainSandRoughness = -1;
static GLint g_uTerrainRockBaseColor = -1;
static GLint g_uTerrainRockMetallic = -1;
static GLint g_uTerrainRockRoughness = -1;
static GLint g_uTerrainNormalStrength = -1;
static GLint g_uTerrainDebugMaterials = -1;
static GLint g_uTerrainSandNormalMap = -1;
static GLint g_uTerrainRockNormalMap = -1;
static GLint g_uTerrainSpotPos = -1;
static GLint g_uTerrainSpotDir = -1;
static GLint g_uTerrainSpotColor = -1;
static GLint g_uTerrainSpotInner = -1;
static GLint g_uTerrainSpotOuter = -1;
static GLint g_uTerrainSpotIntensity = -1;
static GLint g_uTerrainExposure = -1;

static GLint g_uWaterViewProj = -1;
static GLint g_uWaterTime = -1;
static GLint g_uWaterSurfaceLevel = -1;
static GLint g_uWaterCameraY = -1;
static GLint g_uWaterCameraPos = -1;
static GLint g_uWaterLightDir = -1;
static GLint g_uWaterLightColor = -1;
static GLint g_uWaterSpotPos = -1;
static GLint g_uWaterSpotDir = -1;
static GLint g_uWaterSpotColor = -1;
static GLint g_uWaterSpotInner = -1;
static GLint g_uWaterSpotOuter = -1;
static GLint g_uWaterSpotIntensity = -1;
static GLint g_uWaterExposure = -1;

static Camera g_camera;
static CameraInput g_input;
static float g_time = 0.0f;
static float g_waterLevel = 1.4f;
static float g_fogDensity = 0.09f;

struct Material {
    float baseColor[3];
    float metallic;
    float roughness;
};

static Material g_sand{{0.92f, 0.82f, 0.48f}, 0.0f, 0.85f};
static Material g_rock{{0.35f, 0.32f, 0.28f}, 0.05f, 0.72f};
static int g_activeMaterial = 0;
static float g_normalStrength = 1.0f;
static bool g_debugMaterials = false;
static bool g_spotlightEnabled = true;
static bool g_thirdPerson = false;
static bool g_volumetricEnabled = true;
static bool g_hudEnabled = true;
static float g_volumetricStrength = 2.0f;
static int g_highlightedSeaweed = -1;
static int g_collectedSeaweedSamples = 0;

static constexpr float kChaseDistance = 5.5f;

struct SceneLighting {
    Vec3 moonColor;
    Vec3 ambient;
    float fogDensity;
    float exposure;
    float skyBrightness;
    float volStrength;
    Vec3 clearColor;
};

static SceneLighting computeSceneLighting() {
    SceneLighting l{};
    l.moonColor = vec3Scale(kMoonLightColor, 0.30f);
    l.ambient = vec3Scale(kNightAmbient, 0.75f);
    l.fogDensity = g_fogDensity;
    l.exposure = 0.62f;
    l.skyBrightness = 0.45f;
    l.volStrength = g_volumetricStrength;
    l.clearColor = kNightFogColor;
    if (!g_volumetricEnabled) {
        l.volStrength = 0.0f;
    }
    return l;
}

static unsigned char encodeNormal(float v) {
    return static_cast<unsigned char>(clampf(v * 0.5f + 0.5f, 0.0f, 1.0f) * 255.0f + 0.5f);
}

static GLuint createTerrainNormalMap(bool rock) {
    constexpr int size = 128;
    std::vector<unsigned char> pixels(static_cast<size_t>(size * size * 3));

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const float u = static_cast<float>(x) / static_cast<float>(size);
            const float v = static_cast<float>(y) / static_cast<float>(size);

            float nx = 0.0f;
            float ny = 0.0f;
            if (rock) {
                nx = std::sin(u * 42.0f + std::cos(v * 19.0f) * 2.4f) * 0.42f;
                ny = std::cos(v * 38.0f + std::sin(u * 17.0f) * 2.0f) * 0.40f;
                nx += std::sin((u + v) * 85.0f) * 0.12f;
                ny += std::cos((u - v) * 73.0f) * 0.10f;
            } else {
                nx = std::sin(u * 28.0f + v * 11.0f) * 0.18f;
                ny = std::cos(v * 31.0f - u * 8.0f) * 0.16f;
                nx += std::sin((u + v) * 55.0f) * 0.05f;
                ny += std::cos((u - v) * 49.0f) * 0.04f;
            }

            const float z = std::sqrt(std::max(0.08f, 1.0f - nx * nx - ny * ny));
            const size_t idx = static_cast<size_t>((y * size + x) * 3);
            pixels[idx + 0] = encodeNormal(nx);
            pixels[idx + 1] = encodeNormal(ny);
            pixels[idx + 2] = encodeNormal(z);
        }
    }

    GLuint tex = 0;
    glGenTextures_(1, &tex);
    glBindTexture_(kGL_TEXTURE_2D, tex);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_MIN_FILTER, kGL_LINEAR);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_MAG_FILTER, kGL_LINEAR);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D_(kGL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture_(kGL_TEXTURE_2D, 0);
    return tex;
}

static constexpr float kChaseHeight = 2.4f;
static constexpr float kSpotForward = 0.95f;
static constexpr float kSpotUp = 0.12f;
// --- Submarine MODEL orientation (edit here; spotlight is separate) ---
// Axis: {1,0,0}=pitch X, {0,1,0}=yaw Y, {0,0,1}=roll Z. Angle in radians.
// Examples: -PI/2 on X, PI on Y, PI/2 on X + PI on Y via quatMultiply.
static const Quat kSubMeshAxisFix = quatMultiply(
    quatMultiply(
        quatFromAxisAngle({0.0f, 1.0f, 0.0f}, 3.14159265f * 1.5f),
        quatFromAxisAngle({0.0f, 0.0f, 1.0f}, 3.14159265f)
    ),
    quatFromAxisAngle({1.0f, 0.0f, 0.0f}, 3.14159265f)
);
// Model scale: kSubMeshAxisFix block above; also kSubmarineScale (line ~33).
// Vertical centering: submarineModelMatrix() uses (minY+maxY)*0.5f offset.

static Vec3 submarineWorldPos(const Camera &camera) {
    return {camera.x, camera.y, camera.z};
}

static Quat submarineOrientation() {
    return quatNormalize(quatMultiply(g_camera.orientation, kSubMeshAxisFix));
}

// Spotlight follows camera / movement direction (not mesh axis fix).
static void submarineSpotlight(const Vec3 &worldPos, Vec3 &outPos, Vec3 &outDir) {
    const Vec3 forward = cameraForward(g_camera);
    const Vec3 up = quatRotate(g_camera.orientation, {0.0f, 1.0f, 0.0f});
    outPos = vec3Add(worldPos, vec3Add(vec3Scale(forward, kSpotForward), vec3Scale(up, kSpotUp)));
    outDir = vec3Normalize(vec3Add(forward, {0.0f, -0.22f, 0.0f}));
}

static Mat4 submarineModelMatrix(const Vec3 &worldPos) {
    const float centerY = (g_submarine.minY + g_submarine.maxY) * 0.5f;
    const Mat4 body = mat4ModelQuat(worldPos, submarineOrientation(), kSubmarineScale);
    const Mat4 center = mat4Translation({0.0f, -centerY, 0.0f});
    return mat4Multiply(body, center);
}

static void setThirdPersonMode(bool enabled) {
    if (g_thirdPerson == enabled) return;
    g_thirdPerson = enabled;
    invalidateHudCache(g_hud);
}

static int findNearestSeaweedSample(Vec3 pos, float maxDistance) {
    if (!g_seaweed.loaded) return -1;

    int nearest = -1;
    float bestDist2 = maxDistance * maxDistance;
    for (size_t i = 0; i < g_seaweed.instances.size(); ++i) {
        const SeaweedInstance &inst = g_seaweed.instances[i];
        if (inst.collected) continue;

        const float dx = inst.x - pos.x;
        const float dy = inst.y - pos.y;
        const float dz = inst.z - pos.z;
        const float dist2 = dx * dx + dy * dy * 0.35f + dz * dz;
        if (dist2 < bestDist2) {
            bestDist2 = dist2;
            nearest = static_cast<int>(i);
        }
    }
    return nearest;
}

static void collectNearestSeaweedSample() {
    const Vec3 subPos = submarineWorldPos(g_camera);
    const int nearest = findNearestSeaweedSample(subPos, 5.0f);
    g_highlightedSeaweed = nearest;
    if (nearest < 0) return;

    g_seaweed.instances[static_cast<size_t>(nearest)].collected = true;
    ++g_collectedSeaweedSamples;
}

static void buildViewCamera(const Vec3 &subPos, Vec3 &outEye, Vec3 &outTarget) {
    const Vec3 forward = cameraForward(g_camera);
    if (!g_thirdPerson) {
        outEye = subPos;
        outTarget = vec3Add(subPos, forward);
        return;
    }

    const Vec3 back = vec3Scale(forward, -kChaseDistance);
    outEye = vec3Add(vec3Add(subPos, back), Vec3{0.0f, kChaseHeight, 0.0f});
    outTarget = vec3Add(subPos, vec3Scale(forward, 1.2f));
}

static void initPrograms() {
    g_terrainProgram = createProgramFromFiles("shaders/terrain.vert", "shaders/terrain.frag");
    g_uTerrainViewProj = glGetUniformLocation_(g_terrainProgram, "uViewProj");
    g_uTerrainLightVP = glGetUniformLocation_(g_terrainProgram, "uLightVP");
    g_uTerrainTime = glGetUniformLocation_(g_terrainProgram, "uTime");
    g_uTerrainWaterLevel = glGetUniformLocation_(g_terrainProgram, "uWaterLevel");
    g_uTerrainFogDensity = glGetUniformLocation_(g_terrainProgram, "uFogDensity");
    g_uTerrainCameraPos = glGetUniformLocation_(g_terrainProgram, "uCameraPos");
    g_uTerrainDeepColor = glGetUniformLocation_(g_terrainProgram, "uDeepColor");
    g_uTerrainLightDir = glGetUniformLocation_(g_terrainProgram, "uLightDir");
    g_uTerrainLightColor = glGetUniformLocation_(g_terrainProgram, "uLightColor");
    g_uTerrainAmbient = glGetUniformLocation_(g_terrainProgram, "uAmbient");
    g_uTerrainShadowMap = glGetUniformLocation_(g_terrainProgram, "uShadowMap");
    g_uTerrainSandBaseColor = glGetUniformLocation_(g_terrainProgram, "uSandBaseColor");
    g_uTerrainSandMetallic = glGetUniformLocation_(g_terrainProgram, "uSandMetallic");
    g_uTerrainSandRoughness = glGetUniformLocation_(g_terrainProgram, "uSandRoughness");
    g_uTerrainRockBaseColor = glGetUniformLocation_(g_terrainProgram, "uRockBaseColor");
    g_uTerrainRockMetallic = glGetUniformLocation_(g_terrainProgram, "uRockMetallic");
    g_uTerrainRockRoughness = glGetUniformLocation_(g_terrainProgram, "uRockRoughness");
    g_uTerrainNormalStrength = glGetUniformLocation_(g_terrainProgram, "uNormalStrength");
    g_uTerrainDebugMaterials = glGetUniformLocation_(g_terrainProgram, "uDebugMaterials");
    g_uTerrainSandNormalMap = glGetUniformLocation_(g_terrainProgram, "uSandNormalMap");
    g_uTerrainRockNormalMap = glGetUniformLocation_(g_terrainProgram, "uRockNormalMap");
    g_uTerrainSpotPos = glGetUniformLocation_(g_terrainProgram, "uSpotPos");
    g_uTerrainSpotDir = glGetUniformLocation_(g_terrainProgram, "uSpotDir");
    g_uTerrainSpotColor = glGetUniformLocation_(g_terrainProgram, "uSpotColor");
    g_uTerrainSpotInner = glGetUniformLocation_(g_terrainProgram, "uSpotInner");
    g_uTerrainSpotOuter = glGetUniformLocation_(g_terrainProgram, "uSpotOuter");
    g_uTerrainSpotIntensity = glGetUniformLocation_(g_terrainProgram, "uSpotIntensity");
    g_uTerrainExposure = glGetUniformLocation_(g_terrainProgram, "uExposure");

    g_waterProgram = createProgramFromFiles("shaders/water.vert", "shaders/water.frag");
    g_uWaterViewProj = glGetUniformLocation_(g_waterProgram, "uViewProj");
    g_uWaterTime = glGetUniformLocation_(g_waterProgram, "uTime");
    g_uWaterSurfaceLevel = glGetUniformLocation_(g_waterProgram, "uWaterSurfaceLevel");
    g_uWaterCameraY = glGetUniformLocation_(g_waterProgram, "uWaterCameraY");
    g_uWaterCameraPos = glGetUniformLocation_(g_waterProgram, "uCameraPos");
    g_uWaterLightDir = glGetUniformLocation_(g_waterProgram, "uLightDir");
    g_uWaterLightColor = glGetUniformLocation_(g_waterProgram, "uLightColor");
    g_uWaterSpotPos = glGetUniformLocation_(g_waterProgram, "uSpotPos");
    g_uWaterSpotDir = glGetUniformLocation_(g_waterProgram, "uSpotDir");
    g_uWaterSpotColor = glGetUniformLocation_(g_waterProgram, "uSpotColor");
    g_uWaterSpotInner = glGetUniformLocation_(g_waterProgram, "uSpotInner");
    g_uWaterSpotOuter = glGetUniformLocation_(g_waterProgram, "uSpotOuter");
    g_uWaterSpotIntensity = glGetUniformLocation_(g_waterProgram, "uSpotIntensity");
    g_uWaterExposure = glGetUniformLocation_(g_waterProgram, "uExposure");
}

static void initScene() {
    buildTerrain(g_terrain);
    buildWater(g_water);
    initSkybox(g_skybox);
    initShadow(g_shadow, 2048);
    initPrograms();
    g_sandNormalMap = createTerrainNormalMap(false);
    g_rockNormalMap = createTerrainNormalMap(true);
    initSeaweed(g_seaweed, g_waterLevel);
    initRocks(g_rocks, g_waterLevel);
    initSubmarine(g_submarine);
    initFish(g_fish, g_waterLevel);
    initVolumetric(g_volumetric);
    initParticles(g_particles);
    initHud(g_hud);
    glEnable(kGL_DEPTH_TEST);
}

static void resizeViewport(int width, int height) {
    if (height <= 0) height = 1;
    if (glViewport_) glViewport_(0, 0, width, height);
    else glViewport(0, 0, width, height);
}

static void updateInput(float dt) {
    g_input.forward = (GetAsyncKeyState('W') & 0x8000) != 0;
    g_input.backward = (GetAsyncKeyState('S') & 0x8000) != 0;
    g_input.left = (GetAsyncKeyState('A') & 0x8000) != 0;
    g_input.right = (GetAsyncKeyState('D') & 0x8000) != 0;
    g_input.down = (GetAsyncKeyState('Q') & 0x8000) != 0;
    g_input.up = (GetAsyncKeyState('E') & 0x8000) != 0;
    g_input.turnLeft = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
    g_input.turnRight = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
    g_input.turnUp = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
    g_input.turnDown = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;

    updateCamera(g_camera, g_input, dt, terrainHeight, g_time, kTerrainWidth, kTerrainDepth, kCameraClearance, g_waterLevel, kSurfaceClearance, kMoveSpeed, kVerticalSpeed, kTurnSpeed);

    if (GetAsyncKeyState(VK_PRIOR) & 0x8000) g_waterLevel += 1.2f * dt;
    if (GetAsyncKeyState(VK_NEXT) & 0x8000) g_waterLevel -= 1.2f * dt;
    if (GetAsyncKeyState('Z') & 0x8000) g_fogDensity = clampf(g_fogDensity - 0.3f * dt, 0.01f, 0.4f);
    if (GetAsyncKeyState('X') & 0x8000) g_fogDensity = clampf(g_fogDensity + 0.3f * dt, 0.01f, 0.4f);
    if (GetAsyncKeyState('Y') & 0x8000) g_volumetricStrength = clampf(g_volumetricStrength + 0.5f * dt, 0.0f, 5.0f);
    if (GetAsyncKeyState('U') & 0x8000) g_volumetricStrength = clampf(g_volumetricStrength - 0.5f * dt, 0.0f, 5.0f);
}

static void renderFrame() {
    RECT rc{};
    GetClientRect(g_hwnd, &rc);
    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;
    resizeViewport(width, height);

    const Vec3 subPos = submarineWorldPos(g_camera);
    Vec3 eye{};
    Vec3 target{};
    buildViewCamera(subPos, eye, target);

    Vec3 spotPos{};
    Vec3 spotDir{};
    submarineSpotlight(subPos, spotPos, spotDir);
    const Vec3 spotColor = g_spotlightEnabled ? Vec3{0.82f, 0.90f, 1.0f} : Vec3{0.0f, 0.0f, 0.0f};
    const float spotInner = std::cos(16.0f * 3.14159265f / 180.0f);
    const float spotOuter = std::cos(28.0f * 3.14159265f / 180.0f);
    const float spotIntensity = g_spotlightEnabled ? 14.0f : 0.0f;
    const SceneLighting scene = computeSceneLighting();
    const Mat4 proj = mat4Perspective(60.0f * 3.14159265f / 180.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 200.0f);
    const Mat4 view = mat4LookAt(eye, target, {0.0f, 1.0f, 0.0f});
    const Mat4 viewSky = mat4WithoutTranslation(view);
    const Mat4 vp = mat4Multiply(proj, view);
    const Mat4 invViewProj = mat4Inverse(vp);
    const Mat4 lightVP = computeMoonLightMatrix({0.0f, -5.0f, 0.0f}, 34.0f);

    beginShadowPass(g_shadow, lightVP, g_time);
    drawShadowTerrain(g_shadow, g_terrain.vao, g_terrain.indexCount);
    endShadowPass(width, height);

    glClearColor(scene.clearColor.x, scene.clearColor.y, scene.clearColor.z, 1.0f);
    glClear(kGL_COLOR_BUFFER_BIT | kGL_DEPTH_BUFFER_BIT);

    drawSkybox(g_skybox, viewSky.m, proj.m, scene.skyBrightness);

    glUseProgram_(g_terrainProgram);
    glUniformMatrix4fv_(g_uTerrainViewProj, 1, 0, vp.m);
    glUniformMatrix4fv_(g_uTerrainLightVP, 1, 0, lightVP.m);
    glUniform1f_(g_uTerrainTime, g_time);
    glUniform1f_(g_uTerrainWaterLevel, g_waterLevel);
    glUniform1f_(g_uTerrainFogDensity, scene.fogDensity);
    glUniform3f_(g_uTerrainCameraPos, eye.x, eye.y, eye.z);
    glUniform3f_(g_uTerrainDeepColor, scene.clearColor.x, scene.clearColor.y, scene.clearColor.z);
    glUniform3f_(g_uTerrainSandBaseColor, g_sand.baseColor[0], g_sand.baseColor[1], g_sand.baseColor[2]);
    glUniform1f_(g_uTerrainSandMetallic, g_sand.metallic);
    glUniform1f_(g_uTerrainSandRoughness, g_sand.roughness);
    glUniform3f_(g_uTerrainRockBaseColor, g_rock.baseColor[0], g_rock.baseColor[1], g_rock.baseColor[2]);
    glUniform1f_(g_uTerrainRockMetallic, g_rock.metallic);
    glUniform1f_(g_uTerrainRockRoughness, g_rock.roughness);
    glUniform1f_(g_uTerrainNormalStrength, g_normalStrength);
    glUniform1f_(g_uTerrainDebugMaterials, g_debugMaterials ? 1.0f : 0.0f);
    glUniform3f_(g_uTerrainLightDir, kMoonLightDir.x, kMoonLightDir.y, kMoonLightDir.z);
    glUniform3f_(g_uTerrainLightColor, scene.moonColor.x, scene.moonColor.y, scene.moonColor.z);
    glUniform3f_(g_uTerrainAmbient, scene.ambient.x, scene.ambient.y, scene.ambient.z);
    glUniform3f_(g_uTerrainSpotPos, spotPos.x, spotPos.y, spotPos.z);
    glUniform3f_(g_uTerrainSpotDir, spotDir.x, spotDir.y, spotDir.z);
    glUniform3f_(g_uTerrainSpotColor, spotColor.x, spotColor.y, spotColor.z);
    glUniform1f_(g_uTerrainSpotInner, spotInner);
    glUniform1f_(g_uTerrainSpotOuter, spotOuter);
    glUniform1f_(g_uTerrainSpotIntensity, spotIntensity);
    glUniform1f_(g_uTerrainExposure, scene.exposure);
    glActiveTexture_(kGL_TEXTURE0 + 2);
    glBindTexture_(kGL_TEXTURE_2D, g_sandNormalMap);
    glUniform1i_(g_uTerrainSandNormalMap, 2);
    glActiveTexture_(kGL_TEXTURE0 + 3);
    glBindTexture_(kGL_TEXTURE_2D, g_rockNormalMap);
    glUniform1i_(g_uTerrainRockNormalMap, 3);
    glActiveTexture_(kGL_TEXTURE0);
    glBindTexture_(kGL_TEXTURE_2D, g_shadow.depthMap);
    glUniform1i_(g_uTerrainShadowMap, 0);
    glBindVertexArray_(g_terrain.vao);
    glDrawElements_(kGL_TRIANGLES, static_cast<GLsizei>(g_terrain.indexCount), kGL_UNSIGNED_INT, nullptr);
    glBindVertexArray_(0);

    drawSeaweed(g_seaweed, vp, g_time, g_waterLevel, scene.fogDensity, spotPos, spotDir, spotColor, spotInner, spotOuter, spotIntensity, scene.exposure, g_highlightedSeaweed);
    drawRocks(g_rocks, vp, eye, g_waterLevel, scene.fogDensity, kMoonLightDir, scene.moonColor, spotPos, spotDir, spotColor, spotInner, spotOuter, spotIntensity, scene.exposure);

    drawFish(g_fish, vp, g_time, eye, g_waterLevel, scene.fogDensity, kMoonLightDir, scene.moonColor, spotPos, spotDir, spotColor, spotInner, spotOuter, spotIntensity, scene.exposure, scene.clearColor);

    if (g_thirdPerson && g_submarine.loaded) {
        const Mat4 subModel = submarineModelMatrix(subPos);
        drawSubmarine(g_submarine, vp, subModel, eye, g_waterLevel, scene.fogDensity, kMoonLightDir, scene.moonColor, spotPos, spotDir, spotColor, spotInner, spotOuter, spotIntensity, scene.exposure);
    }

    const Vec3 volMoonColor = scene.moonColor;

    glEnable(kGL_BLEND);
    glBlendFunc(kGL_SRC_ALPHA, kGL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(0);
    glUseProgram_(g_waterProgram);
    glUniformMatrix4fv_(g_uWaterViewProj, 1, 0, vp.m);
    glUniform1f_(g_uWaterTime, g_time);
    glUniform1f_(g_uWaterSurfaceLevel, g_waterLevel);
    glUniform1f_(g_uWaterCameraY, eye.y);
    glUniform3f_(g_uWaterCameraPos, eye.x, eye.y, eye.z);
    glUniform3f_(g_uWaterLightDir, kMoonLightDir.x, kMoonLightDir.y, kMoonLightDir.z);
    glUniform3f_(g_uWaterLightColor, scene.moonColor.x, scene.moonColor.y, scene.moonColor.z);
    glUniform3f_(g_uWaterSpotPos, spotPos.x, spotPos.y, spotPos.z);
    glUniform3f_(g_uWaterSpotDir, spotDir.x, spotDir.y, spotDir.z);
    glUniform3f_(g_uWaterSpotColor, spotColor.x, spotColor.y, spotColor.z);
    glUniform1f_(g_uWaterSpotInner, spotInner);
    glUniform1f_(g_uWaterSpotOuter, spotOuter);
    glUniform1f_(g_uWaterSpotIntensity, spotIntensity);
    glUniform1f_(g_uWaterExposure, scene.exposure);
    glBindVertexArray_(g_water.vao);
    glDrawArrays(kGL_TRIANGLES, 0, static_cast<GLsizei>(g_water.count));
    glBindVertexArray_(0);
    glUseProgram_(0);
    glDepthMask(1);
    glDisable(kGL_BLEND);

    if (scene.volStrength > 0.0001f) {
        captureVolumetricDepth(g_volumetric, width, height);
        drawVolumetric(g_volumetric, invViewProj, eye, g_waterLevel, scene.fogDensity, scene.volStrength, g_time, kMoonLightDir, volMoonColor, spotPos, spotDir, spotColor, spotInner, spotOuter, spotIntensity);
    }
    drawParticles(g_particles, vp, eye, g_waterLevel, g_time);
    if (g_hudEnabled) {
        drawHud(g_hud, width, height, g_thirdPerson, g_spotlightEnabled, g_volumetricEnabled,
            g_volumetricStrength, g_collectedSeaweedSamples, static_cast<int>(g_seaweed.instances.size()));
    }

    SwapBuffers(g_hdc);
}

static Material &activeMaterial() { return g_activeMaterial == 0 ? g_sand : g_rock; }

static void handleMaterialKey(WPARAM key) {
    Material &m = activeMaterial();
    switch (key) {
    case '1': g_activeMaterial = 0; break;
    case '2': g_activeMaterial = 1; break;
    case 'R': m.roughness = clampf(m.roughness + 0.05f, 0.02f, 1.0f); break;
    case 'F': m.roughness = clampf(m.roughness - 0.05f, 0.02f, 1.0f); break;
    case 'M': m.metallic = clampf(m.metallic + 0.05f, 0.0f, 1.0f); break;
    case 'N': m.metallic = clampf(m.metallic - 0.05f, 0.0f, 1.0f); break;
    case 'B': g_normalStrength = clampf(g_normalStrength + 0.15f, 0.0f, 3.0f); break;
    case 'V': g_normalStrength = clampf(g_normalStrength - 0.15f, 0.0f, 3.0f); break;
    case 'H': g_debugMaterials = !g_debugMaterials; break;
    case 'G':
        collectNearestSeaweedSample();
        break;
    case 'T':
        setThirdPersonMode(!g_thirdPerson);
        break;
    case 'P':
        cycleFishDisplayMode(g_fish);
        break;
    case 'L':
    case VK_F1:
        g_spotlightEnabled = !g_spotlightEnabled;
        break;
    case VK_F2:
        g_hudEnabled = !g_hudEnabled;
        if (g_hudEnabled) {
            invalidateHudCache(g_hud);
        }
        break;
    case 'Y':
        g_volumetricStrength = clampf(g_volumetricStrength + 0.15f, 0.0f, 5.0f);
        break;
    case 'U':
        g_volumetricStrength = clampf(g_volumetricStrength - 0.15f, 0.0f, 5.0f);
        break;
    case 'O':
        g_volumetricEnabled = !g_volumetricEnabled;
        break;
    case 'C':
        g_camera.x = 0.0f;
        g_camera.y = -10.0f;
        g_camera.z = 11.0f;
        g_camera.orientation = quatFromAxisAngle({0.0f, 1.0f, 0.0f}, 3.14159265f);
        break;
    default: return;
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        resizeViewport(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_KEYDOWN:
        if (lParam & 0x40000000) return 0;
        switch (wParam) {
        case VK_ESCAPE:
            g_running = false;
            PostQuitMessage(0);
            break;
        default:
            handleMaterialKey(static_cast<WPARAM>(wParam));
            break;
        }
        return 0;
    case WM_KEYUP:
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

static bool initOpenGL(HWND hwnd) {
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    g_hdc = GetDC(hwnd);
    const int pf = ChoosePixelFormat(g_hdc, &pfd);
    if (!pf || !SetPixelFormat(g_hdc, pf, &pfd)) return false;

    HGLRC temp = wglCreateContext(g_hdc);
    if (!temp || !wglMakeCurrent(g_hdc, temp)) return false;
    if (!loadGLFunctions()) return false;
    initScene();
    g_hrc = temp;
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSA wc{};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "UnderwaterWorldWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    RECT rect{0, 0, kWidth, kHeight};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    g_hwnd = CreateWindowA(wc.lpszClassName, "", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr);
    if (!g_hwnd) return 1;
    if (!initOpenGL(g_hwnd)) return 2;
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    SetFocus(g_hwnd);

    MSG msg{};
    ULONGLONG last = GetTickCount64();
    while (g_running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!g_running) break;
        const ULONGLONG now = GetTickCount64();
        float dt = static_cast<float>(now - last) / 1000.0f;
        last = now;
        dt = clampf(dt, 0.0f, 0.033f);
        g_time += dt;
        updateInput(dt);
        updateFish(g_fish, dt);
        renderFrame();
        Sleep(1);
    }

    if (g_terrainProgram) glDeleteProgram_(g_terrainProgram);
    if (g_waterProgram) glDeleteProgram_(g_waterProgram);
    if (g_sandNormalMap) glDeleteTextures_(1, &g_sandNormalMap);
    if (g_rockNormalMap) glDeleteTextures_(1, &g_rockNormalMap);
    destroySkybox(g_skybox);
    destroyShadow(g_shadow);
    destroySeaweed(g_seaweed);
    destroyRocks(g_rocks);
    destroySubmarine(g_submarine);
    destroyFish(g_fish);
    destroyVolumetric(g_volumetric);
    destroyParticles(g_particles);
    destroyHud(g_hud);
    destroyWater(g_water);
    destroyTerrain(g_terrain);
    if (g_hrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(g_hrc);
    }
    if (g_hwnd && g_hdc) ReleaseDC(g_hwnd, g_hdc);
    return 0;
}

int main() {
    return WinMain(GetModuleHandleA(nullptr), nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
}
