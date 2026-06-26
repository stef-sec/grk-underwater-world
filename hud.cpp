#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "hud.h"

#include "shader.h"

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <vector>

struct HudVertex {
    float x, y;
    float r, g, b, a;
};

static const char *glyphRows(char c) {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c)))) {
    case 'A': return "01110""10001""10001""11111""10001""10001""10001";
    case 'B': return "11110""10001""10001""11110""10001""10001""11110";
    case 'C': return "01111""10000""10000""10000""10000""10000""01111";
    case 'D': return "11110""10001""10001""10001""10001""10001""11110";
    case 'E': return "11111""10000""10000""11110""10000""10000""11111";
    case 'F': return "11111""10000""10000""11110""10000""10000""10000";
    case 'G': return "01111""10000""10000""10011""10001""10001""01111";
    case 'H': return "10001""10001""10001""11111""10001""10001""10001";
    case 'I': return "11111""00100""00100""00100""00100""00100""11111";
    case 'J': return "00111""00010""00010""00010""10010""10010""01100";
    case 'K': return "10001""10010""10100""11000""10100""10010""10001";
    case 'L': return "10000""10000""10000""10000""10000""10000""11111";
    case 'M': return "10001""11011""10101""10101""10001""10001""10001";
    case 'N': return "10001""11001""10101""10011""10001""10001""10001";
    case 'O': return "01110""10001""10001""10001""10001""10001""01110";
    case 'P': return "11110""10001""10001""11110""10000""10000""10000";
    case 'Q': return "01110""10001""10001""10001""10101""10010""01101";
    case 'R': return "11110""10001""10001""11110""10100""10010""10001";
    case 'S': return "01111""10000""10000""01110""00001""00001""11110";
    case 'T': return "11111""00100""00100""00100""00100""00100""00100";
    case 'U': return "10001""10001""10001""10001""10001""10001""01110";
    case 'V': return "10001""10001""10001""10001""10001""01010""00100";
    case 'W': return "10001""10001""10001""10101""10101""10101""01010";
    case 'X': return "10001""10001""01010""00100""01010""10001""10001";
    case 'Y': return "10001""10001""01010""00100""00100""00100""00100";
    case 'Z': return "11111""00001""00010""00100""01000""10000""11111";
    case '0': return "01110""10001""10011""10101""11001""10001""01110";
    case '1': return "00100""01100""00100""00100""00100""00100""01110";
    case '2': return "01110""10001""00001""00010""00100""01000""11111";
    case '3': return "11110""00001""00001""01110""00001""00001""11110";
    case '4': return "00010""00110""01010""10010""11111""00010""00010";
    case '5': return "11111""10000""10000""11110""00001""00001""11110";
    case '6': return "01111""10000""10000""11110""10001""10001""01110";
    case '7': return "11111""00001""00010""00100""01000""01000""01000";
    case '8': return "01110""10001""10001""01110""10001""10001""01110";
    case '9': return "01110""10001""10001""01111""00001""00001""11110";
    case ':': return "00000""00100""00100""00000""00100""00100""00000";
    case '.': return "00000""00000""00000""00000""00000""01100""01100";
    case '/': return "00001""00010""00010""00100""01000""01000""10000";
    case '|': return "00100""00100""00100""00100""00100""00100""00100";
    case ',': return "00000""00000""00000""00000""00100""00100""01000";
    case '-': return "00000""00000""00000""11111""00000""00000""00000";
    default: return nullptr;
    }
}

static float pxToNdcX(float px, float width) {
    return px / width * 2.0f - 1.0f;
}

static float pxToNdcY(float py, float height) {
    return 1.0f - py / height * 2.0f;
}

static void pushQuadNdc(std::vector<HudVertex> &v, float px, float py, float pw, float ph,
    float sw, float sh, float r, float g, float b, float a) {
    const float x0 = pxToNdcX(px, sw);
    const float x1 = pxToNdcX(px + pw, sw);
    const float y0 = pxToNdcY(py, sh);
    const float y1 = pxToNdcY(py + ph, sh);
    v.push_back({x0, y0, r, g, b, a});
    v.push_back({x1, y0, r, g, b, a});
    v.push_back({x1, y1, r, g, b, a});
    v.push_back({x0, y0, r, g, b, a});
    v.push_back({x1, y1, r, g, b, a});
    v.push_back({x0, y1, r, g, b, a});
}

static void pushTextNdc(std::vector<HudVertex> &v, const char *text, float px, float py, float scale,
    float sw, float sh, float r, float g, float b, float a) {
    const float startX = px;
    const float pixel = scale;
    const float advance = 6.0f * scale;
    for (const char *p = text; *p; ++p) {
        if (*p == '\n') {
            px = startX;
            py += 9.0f * scale;
            continue;
        }
        if (*p == ' ') {
            px += advance;
            continue;
        }
        const char *rows = glyphRows(*p);
        if (rows) {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (rows[row * 5 + col] == '1') {
                        pushQuadNdc(v, px + col * pixel, py + row * pixel, pixel, pixel, sw, sh, r, g, b, a);
                    }
                }
            }
        } else {
            pushQuadNdc(v, px, py + 2.0f * pixel, 4.0f * pixel, 3.0f * pixel, sw, sh, r, g, b, a);
        }
        px += advance;
    }
}

void invalidateHudCache(HudGPU &hud) {
    hud.cachedWidth = -1;
    hud.cachedHeight = -1;
    hud.cachedCollectedSamples = -1;
    hud.cachedTotalSamples = -1;
    hud.cachedVolumetricStrength = -1.0f;
}

void initHud(HudGPU &hud) {
    hud.program = createProgramFromFiles("shaders/hud.vert", "shaders/hud.frag");
    if (!hud.program) return;

    glGenVertexArrays_(1, &hud.vao);
    glBindVertexArray_(hud.vao);
    glGenBuffers_(1, &hud.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, hud.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, 1, nullptr, kGL_DYNAMIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 2, kGL_FLOAT, 0, sizeof(HudVertex), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray_(1);
    glVertexAttribPointer_(1, 4, kGL_FLOAT, 0, sizeof(HudVertex), reinterpret_cast<void *>(offsetof(HudVertex, r)));
    glBindVertexArray_(0);
}

void drawHud(HudGPU &hud, int width, int height, bool thirdPerson, bool spotlightEnabled,
    bool volumetricEnabled, float volumetricStrength, int collectedSamples, int totalSamples) {
    if (!hud.program || width <= 0 || height <= 0) return;

    const float sw = static_cast<float>(width);
    const float sh = static_cast<float>(height);

    const bool dirty =
        hud.cachedWidth != width ||
        hud.cachedHeight != height ||
        hud.cachedThirdPerson != thirdPerson ||
        hud.cachedSpotlightEnabled != spotlightEnabled ||
        hud.cachedVolumetricEnabled != volumetricEnabled ||
        hud.cachedVolumetricStrength != volumetricStrength ||
        hud.cachedCollectedSamples != collectedSamples ||
        hud.cachedTotalSamples != totalSamples;

    if (dirty) {
        char status[768];
        std::snprintf(status, sizeof(status),
            "GRK UNDERWATER WORLD\n"
            "VIEW: %s | SPOTLIGHT: %s | VOLUMETRIC: %s %.1f\n"
            "SEAWEED: %d/%d (%d LEFT ON MAP)\n"
            "WASD MOVE, ARROWS TURN, Q/E VERTICAL, T VIEW, L/F1 LIGHT, O SHAFTS, Y/U STRENGTH, G COLLECT, C RESET, F2 HUD",
            thirdPerson ? "3RD PERSON" : "1ST PERSON",
            spotlightEnabled ? "ON" : "OFF",
            volumetricEnabled ? "ON" : "OFF",
            volumetricStrength,
            collectedSamples,
            totalSamples,
            totalSamples - collectedSamples);

        std::vector<HudVertex> vertices;
        vertices.reserve(32000);
        pushTextNdc(vertices, status, 18.0f, 18.0f, 2.0f, sw, sh, 0.85f, 0.95f, 1.0f, 1.0f);

        glBindVertexArray_(hud.vao);
        glBindBuffer_(kGL_ARRAY_BUFFER, hud.vbo);
        glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(HudVertex)), vertices.data(), kGL_DYNAMIC_DRAW);
        glBindVertexArray_(0);

        hud.vertexCount = static_cast<GLsizei>(vertices.size());
        hud.cachedWidth = width;
        hud.cachedHeight = height;
        hud.cachedThirdPerson = thirdPerson;
        hud.cachedSpotlightEnabled = spotlightEnabled;
        hud.cachedVolumetricEnabled = volumetricEnabled;
        hud.cachedVolumetricStrength = volumetricStrength;
        hud.cachedCollectedSamples = collectedSamples;
        hud.cachedTotalSamples = totalSamples;
    }

    if (hud.vertexCount <= 0) return;

    if (glBindFramebuffer_) glBindFramebuffer_(kGL_FRAMEBUFFER, 0);
    if (glViewport_) glViewport_(0, 0, width, height);

    glDisable(kGL_DEPTH_TEST);
    glDisable(kGL_CULL_FACE);
    glDepthMask(1);
    glEnable(kGL_BLEND);
    glBlendFunc(kGL_SRC_ALPHA, kGL_ONE_MINUS_SRC_ALPHA);
    glUseProgram_(hud.program);
    glBindVertexArray_(hud.vao);
    glDrawArrays(kGL_TRIANGLES, 0, hud.vertexCount);
    glBindVertexArray_(0);
    glUseProgram_(0);
    glDisable(kGL_BLEND);
    glEnable(kGL_DEPTH_TEST);
}

void destroyHud(HudGPU &hud) {
    if (hud.program) glDeleteProgram_(hud.program);
    if (hud.vbo) glDeleteBuffers_(1, &hud.vbo);
    if (hud.vao) glDeleteVertexArrays_(1, &hud.vao);
    hud = {};
}
