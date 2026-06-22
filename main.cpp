#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include <cmath>
#include <cstdint>
#include <cstdio>

#include "camera.h"
#include "gl_loader.h"
#include "lighting.h"
#include "math.h"
#include "props.h"
#include "shader.h"
#include "shadow.h"
#include "skybox.h"
#include "terrain.h"
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
static constexpr float kSurfaceClearance = 0.45f;

static HDC g_hdc = nullptr;
static HGLRC g_hrc = nullptr;
static HWND g_hwnd = nullptr;
static bool g_running = true;

static TerrainGPU g_terrain;
static WaterGPU g_water;
static SkyboxGPU g_skybox;
static ShadowGPU g_shadow;
static SeaweedGPU g_seaweed;

static GLuint g_terrainProgram = 0;
static GLuint g_waterProgram = 0;

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

static GLint g_uWaterViewProj = -1;
static GLint g_uWaterTime = -1;
static GLint g_uWaterSurfaceLevel = -1;
static GLint g_uWaterCameraY = -1;
static GLint g_uWaterCameraPos = -1;
static GLint g_uWaterLightDir = -1;
static GLint g_uWaterLightColor = -1;

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

static void updateWindowTitle() {
    const Material &m = g_activeMaterial == 0 ? g_sand : g_rock;
    const char *name = g_activeMaterial == 0 ? "Piasek" : "Skala";
    char title[512];
    snprintf(title, sizeof(title),
        "GRK Underwater World | Mat: %s (1/2) | Rough=%.2f Metal=%.2f | Normal=%.2f | H debug | C close-up",
        name, m.roughness, m.metallic, g_normalStrength);
    SetWindowTextA(g_hwnd, title);
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

    g_waterProgram = createProgramFromFiles("shaders/water.vert", "shaders/water.frag");
    g_uWaterViewProj = glGetUniformLocation_(g_waterProgram, "uViewProj");
    g_uWaterTime = glGetUniformLocation_(g_waterProgram, "uTime");
    g_uWaterSurfaceLevel = glGetUniformLocation_(g_waterProgram, "uWaterSurfaceLevel");
    g_uWaterCameraY = glGetUniformLocation_(g_waterProgram, "uWaterCameraY");
    g_uWaterCameraPos = glGetUniformLocation_(g_waterProgram, "uCameraPos");
    g_uWaterLightDir = glGetUniformLocation_(g_waterProgram, "uLightDir");
    g_uWaterLightColor = glGetUniformLocation_(g_waterProgram, "uLightColor");
}

static void initScene() {
    buildTerrain(g_terrain);
    buildWater(g_water);
    initSkybox(g_skybox);
    initShadow(g_shadow, 2048);
    initPrograms();
    initSeaweed(g_seaweed, g_waterLevel);
    glEnable(kGL_DEPTH_TEST);
}

static void resizeViewport(int width, int height) {
    if (height <= 0) height = 1;
    if (glViewport_) glViewport_(0, 0, width, height);
    else glViewport(0, 0, width, height);
}

static void updateInput(float dt) {
    updateCamera(g_camera, g_input, dt, terrainHeight, g_time, kTerrainWidth, kTerrainDepth, kCameraClearance, g_waterLevel, kSurfaceClearance, kMoveSpeed, kVerticalSpeed, kTurnSpeed);

    if (GetAsyncKeyState(VK_PRIOR) & 0x8000) g_waterLevel += 1.2f * dt;
    if (GetAsyncKeyState(VK_NEXT) & 0x8000) g_waterLevel -= 1.2f * dt;
    if (GetAsyncKeyState('Z') & 0x8000) g_fogDensity = clampf(g_fogDensity - 0.3f * dt, 0.01f, 0.4f);
    if (GetAsyncKeyState('X') & 0x8000) g_fogDensity = clampf(g_fogDensity + 0.3f * dt, 0.01f, 0.4f);
}

static void renderFrame() {
    RECT rc{};
    GetClientRect(g_hwnd, &rc);
    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;
    resizeViewport(width, height);

    const Vec3 eye{g_camera.x, g_camera.y, g_camera.z};
    const Vec3 forward = cameraForward(g_camera);
    const Vec3 target = vec3Add(eye, forward);
    const Mat4 proj = mat4Perspective(60.0f * 3.14159265f / 180.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 200.0f);
    const Mat4 view = mat4LookAt(eye, target, {0.0f, 1.0f, 0.0f});
    const Mat4 viewSky = mat4WithoutTranslation(view);
    const Mat4 vp = mat4Multiply(proj, view);
    const Mat4 lightVP = computeMoonLightMatrix({0.0f, -5.0f, 0.0f}, 34.0f);

    beginShadowPass(g_shadow, lightVP, g_time);
    drawShadowTerrain(g_shadow, g_terrain.vao, g_terrain.indexCount);
    endShadowPass(width, height);

    glClearColor(kNightFogColor.x, kNightFogColor.y, kNightFogColor.z, 1.0f);
    glClear(kGL_COLOR_BUFFER_BIT | kGL_DEPTH_BUFFER_BIT);

    drawSkybox(g_skybox, viewSky.m, proj.m);

    glUseProgram_(g_terrainProgram);
    glUniformMatrix4fv_(g_uTerrainViewProj, 1, 0, vp.m);
    glUniformMatrix4fv_(g_uTerrainLightVP, 1, 0, lightVP.m);
    glUniform1f_(g_uTerrainTime, g_time);
    glUniform1f_(g_uTerrainWaterLevel, g_waterLevel);
    glUniform1f_(g_uTerrainFogDensity, g_fogDensity);
    glUniform3f_(g_uTerrainCameraPos, g_camera.x, g_camera.y, g_camera.z);
    glUniform3f_(g_uTerrainDeepColor, kNightFogColor.x, kNightFogColor.y, kNightFogColor.z);
    glUniform3f_(g_uTerrainSandBaseColor, g_sand.baseColor[0], g_sand.baseColor[1], g_sand.baseColor[2]);
    glUniform1f_(g_uTerrainSandMetallic, g_sand.metallic);
    glUniform1f_(g_uTerrainSandRoughness, g_sand.roughness);
    glUniform3f_(g_uTerrainRockBaseColor, g_rock.baseColor[0], g_rock.baseColor[1], g_rock.baseColor[2]);
    glUniform1f_(g_uTerrainRockMetallic, g_rock.metallic);
    glUniform1f_(g_uTerrainRockRoughness, g_rock.roughness);
    glUniform1f_(g_uTerrainNormalStrength, g_normalStrength);
    glUniform1f_(g_uTerrainDebugMaterials, g_debugMaterials ? 1.0f : 0.0f);
    glUniform3f_(g_uTerrainLightDir, kMoonLightDir.x, kMoonLightDir.y, kMoonLightDir.z);
    glUniform3f_(g_uTerrainLightColor, kMoonLightColor.x, kMoonLightColor.y, kMoonLightColor.z);
    glUniform3f_(g_uTerrainAmbient, kNightAmbient.x, kNightAmbient.y, kNightAmbient.z);
    glActiveTexture_(kGL_TEXTURE0);
    glBindTexture_(kGL_TEXTURE_2D, g_shadow.depthMap);
    glUniform1i_(g_uTerrainShadowMap, 0);
    glBindVertexArray_(g_terrain.vao);
    glDrawElements_(kGL_TRIANGLES, static_cast<GLsizei>(g_terrain.indexCount), kGL_UNSIGNED_INT, nullptr);
    glBindVertexArray_(0);

    drawSeaweed(g_seaweed, vp, g_time, g_waterLevel, g_fogDensity);

    glEnable(kGL_BLEND);
    glBlendFunc(kGL_SRC_ALPHA, kGL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(0);
    glUseProgram_(g_waterProgram);
    glUniformMatrix4fv_(g_uWaterViewProj, 1, 0, vp.m);
    glUniform1f_(g_uWaterTime, g_time);
    glUniform1f_(g_uWaterSurfaceLevel, g_waterLevel);
    glUniform1f_(g_uWaterCameraY, g_camera.y);
    glUniform3f_(g_uWaterCameraPos, g_camera.x, g_camera.y, g_camera.z);
    glUniform3f_(g_uWaterLightDir, kMoonLightDir.x, kMoonLightDir.y, kMoonLightDir.z);
    glUniform3f_(g_uWaterLightColor, kMoonLightColor.x, kMoonLightColor.y, kMoonLightColor.z);
    glBindVertexArray_(g_water.vao);
    glDrawArrays(kGL_TRIANGLES, 0, static_cast<GLsizei>(g_water.count));
    glBindVertexArray_(0);
    glUseProgram_(0);
    glDepthMask(1);
    glDisable(kGL_BLEND);

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
    case 'C':
        g_camera.x = 0.0f;
        g_camera.y = -3.5f;
        g_camera.z = 8.0f;
        g_camera.orientation = quatFromAxisAngle({0.0f, 1.0f, 0.0f}, 3.14159265f);
        break;
    default: return;
    }
    updateWindowTitle();
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        resizeViewport(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_KEYDOWN:
        switch (wParam) {
        case 'W': g_input.forward = true; break;
        case 'A': g_input.left = true; break;
        case 'S': g_input.backward = true; break;
        case 'D': g_input.right = true; break;
        case 'Q': g_input.down = true; break;
        case 'E': g_input.up = true; break;
        case VK_LEFT: g_input.turnLeft = true; break;
        case VK_RIGHT: g_input.turnRight = true; break;
        case VK_UP: g_input.turnUp = true; break;
        case VK_DOWN: g_input.turnDown = true; break;
        case VK_ESCAPE: g_running = false; PostQuitMessage(0); break;
        case '1':
        case '2':
        case 'R':
        case 'F':
        case 'M':
        case 'N':
        case 'B':
        case 'V':
        case 'H':
        case 'C':
            handleMaterialKey(static_cast<WPARAM>(wParam));
            break;
        default: break;
        }
        return 0;
    case WM_KEYUP:
        switch (wParam) {
        case 'W': g_input.forward = false; break;
        case 'A': g_input.left = false; break;
        case 'S': g_input.backward = false; break;
        case 'D': g_input.right = false; break;
        case 'Q': g_input.down = false; break;
        case 'E': g_input.up = false; break;
        case VK_LEFT: g_input.turnLeft = false; break;
        case VK_RIGHT: g_input.turnRight = false; break;
        case VK_UP: g_input.turnUp = false; break;
        case VK_DOWN: g_input.turnDown = false; break;
        default: break;
        }
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
    g_hwnd = CreateWindowA(wc.lpszClassName, "GRK Underwater World - Night", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr);
    if (!g_hwnd) return 1;
    if (!initOpenGL(g_hwnd)) return 2;
    updateWindowTitle();
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

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
        renderFrame();
        Sleep(1);
    }

    if (g_terrainProgram) glDeleteProgram_(g_terrainProgram);
    if (g_waterProgram) glDeleteProgram_(g_waterProgram);
    destroySkybox(g_skybox);
    destroyShadow(g_shadow);
    destroySeaweed(g_seaweed);
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
