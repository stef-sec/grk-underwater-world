#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "skybox.h"
#include "shader.h"
#include "gl_loader.h"
#include "math.h"
#include "lighting.h"

#include <cmath>
#include <cstdint>
#include <vector>

static float hash21(float x, float y) {
    float n = std::sin(x * 127.1f + y * 311.7f) * 43758.5453f;
    return n - std::floor(n);
}

static Vec3 cubemapColorForDirection(Vec3 dir) {
    dir = vec3Normalize(dir);
    const float up = dir.y;

    if (up > 0.15f) {
        float t = clampf(up, 0.0f, 1.0f);
        Vec3 zenith = {0.01f, 0.02f, 0.06f};
        Vec3 horizon = {0.03f, 0.06f, 0.12f};
        Vec3 col = {
            lerpf(horizon.x, zenith.x, t),
            lerpf(horizon.y, zenith.y, t),
            lerpf(horizon.z, zenith.z, t),
        };

        float starField = hash21(dir.x * 180.0f, dir.z * 180.0f);
        if (starField > 0.992f) col = vec3Add(col, vec3Scale({1.0f, 1.0f, 1.0f}, (starField - 0.992f) * 12.0f));

        Vec3 moonDir = vec3Normalize(kMoonLightDir);
        float moonDot = vec3Dot(dir, moonDir);
        float moonDisc = smoothstep(0.9992f, 0.9999f, moonDot);
        float moonGlow = std::pow(clampf(moonDot, 0.0f, 1.0f), 64.0f) * 0.35f;
        col = vec3Add(col, {0.75f * moonDisc, 0.82f * moonDisc, 0.95f * moonDisc});
        col = vec3Add(col, {0.25f * moonGlow, 0.35f * moonGlow, 0.55f * moonGlow});
        return col;
    }

    float depth = clampf(-up, 0.0f, 1.0f);
    Vec3 shallow = {0.02f, 0.10f, 0.16f};
    Vec3 deep = {0.005f, 0.025f, 0.05f};
    Vec3 col = {
        lerpf(shallow.x, deep.x, depth),
        lerpf(shallow.y, deep.y, depth),
        lerpf(shallow.z, deep.z, depth),
    };

    float moonRay = clampf(vec3Dot(vec3Normalize({dir.x, std::fabs(dir.y) + 0.2f, dir.z}), vec3Normalize(kMoonLightDir)), 0.0f, 1.0f);
    moonRay = std::pow(moonRay, 12.0f) * 0.18f;
    col = vec3Add(col, {0.15f * moonRay, 0.22f * moonRay, 0.35f * moonRay});
    return col;
}

static GLuint createNightCubemap() {
    constexpr int faceSize = 256;
    std::vector<float> pixels(faceSize * faceSize * 3);

    GLuint tex = 0;
    glGenTextures_(1, &tex);
    glBindTexture_(kGL_TEXTURE_CUBE_MAP, tex);

    for (int face = 0; face < 6; ++face) {
        for (int y = 0; y < faceSize; ++y) {
            for (int x = 0; x < faceSize; ++x) {
                float u = (static_cast<float>(x) + 0.5f) / faceSize * 2.0f - 1.0f;
                float v = (static_cast<float>(y) + 0.5f) / faceSize * 2.0f - 1.0f;
                Vec3 dir{};
                switch (face) {
                case 0: dir = {1.0f, -v, -u}; break;
                case 1: dir = {-1.0f, -v, u}; break;
                case 2: dir = {u, 1.0f, v}; break;
                case 3: dir = {u, -1.0f, -v}; break;
                case 4: dir = {u, -v, 1.0f}; break;
                default: dir = {-u, -v, -1.0f}; break;
                }
                Vec3 col = cubemapColorForDirection(dir);
                size_t idx = static_cast<size_t>((y * faceSize + x) * 3);
                pixels[idx + 0] = col.x;
                pixels[idx + 1] = col.y;
                pixels[idx + 2] = col.z;
            }
        }
        glTexImage2D_(kGL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 0x8051, faceSize, faceSize, 0, 0x1907, kGL_FLOAT, pixels.data());
    }

    glTexParameteri_(kGL_TEXTURE_CUBE_MAP, kGL_TEXTURE_MIN_FILTER, kGL_LINEAR);
    glTexParameteri_(kGL_TEXTURE_CUBE_MAP, kGL_TEXTURE_MAG_FILTER, kGL_LINEAR);
    glTexParameteri_(kGL_TEXTURE_CUBE_MAP, kGL_TEXTURE_WRAP_S, kGL_CLAMP_TO_EDGE);
    glTexParameteri_(kGL_TEXTURE_CUBE_MAP, kGL_TEXTURE_WRAP_T, kGL_CLAMP_TO_EDGE);
    glBindTexture_(kGL_TEXTURE_CUBE_MAP, 0);
    return tex;
}

void initSkybox(SkyboxGPU &skybox) {
    const float vertices[] = {
        -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
    };

    skybox.program = createProgramFromFiles("shaders/skybox.vert", "shaders/skybox.frag");
    skybox.uView = glGetUniformLocation_(skybox.program, "uView");
    skybox.uProj = glGetUniformLocation_(skybox.program, "uProj");
    skybox.uCubemap = glGetUniformLocation_(skybox.program, "uCubemap");
    skybox.uBrightness = glGetUniformLocation_(skybox.program, "uBrightness");
    skybox.cubemap = createNightCubemap();

    glGenVertexArrays_(1, &skybox.vao);
    glBindVertexArray_(skybox.vao);
    GLuint vbo = 0;
    glGenBuffers_(1, &vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, vbo);
    glBufferData_(kGL_ARRAY_BUFFER, sizeof(vertices), vertices, kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, 3 * sizeof(float), reinterpret_cast<void *>(0));
    glBindVertexArray_(0);
    skybox.vbo = vbo;
}

void drawSkybox(const SkyboxGPU &skybox, const float viewNoTranslation[16], const float proj[16], float brightness) {
    glDepthFunc(kGL_LEQUAL);
    glUseProgram_(skybox.program);
    glUniformMatrix4fv_(skybox.uView, 1, 0, viewNoTranslation);
    glUniformMatrix4fv_(skybox.uProj, 1, 0, proj);
    glUniform1f_(skybox.uBrightness, brightness);
    glActiveTexture_(kGL_TEXTURE0);
    glBindTexture_(kGL_TEXTURE_CUBE_MAP, skybox.cubemap);
    glUniform1i_(skybox.uCubemap, 0);
    glBindVertexArray_(skybox.vao);
    glDrawArrays(kGL_TRIANGLES, 0, 36);
    glBindVertexArray_(0);
    glDepthFunc(kGL_LESS);
}

void destroySkybox(SkyboxGPU &skybox) {
    if (skybox.program) glDeleteProgram_(skybox.program);
    if (skybox.vbo) glDeleteBuffers_(1, &skybox.vbo);
    if (skybox.vao) glDeleteVertexArrays_(1, &skybox.vao);
    if (skybox.cubemap) glDeleteTextures_(1, &skybox.cubemap);
    skybox = {};
}
