#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "terrain.h"

#include <cmath>
#include <cstdint>
#include <vector>

using GLfloat = float;
using GLsizeiptr = ptrdiff_t;
using GLboolean = unsigned char;

using MyGLGenBuffersProc = void(__stdcall *)(GLsizei, GLuint *);
using MyGLBindBufferProc = void(__stdcall *)(GLenum, GLuint);
using MyGLBufferDataProc = void(__stdcall *)(GLenum, GLsizeiptr, const void *, GLenum);
using MyGLGenVertexArraysProc = void(__stdcall *)(GLsizei, GLuint *);
using MyGLBindVertexArrayProc = void(__stdcall *)(GLuint);
using MyGLEnableVertexAttribArrayProc = void(__stdcall *)(GLuint);
using MyGLVertexAttribPointerProc = void(__stdcall *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);

extern MyGLGenBuffersProc glGenBuffers_;
extern MyGLBindBufferProc glBindBuffer_;
extern MyGLBufferDataProc glBufferData_;
extern MyGLGenVertexArraysProc glGenVertexArrays_;
extern MyGLBindVertexArrayProc glBindVertexArray_;
extern MyGLEnableVertexAttribArrayProc glEnableVertexAttribArray_;
extern MyGLVertexAttribPointerProc glVertexAttribPointer_;

static constexpr GLenum kGL_ARRAY_BUFFER = 0x8892;
static constexpr GLenum kGL_STATIC_DRAW = 0x88E4;
static constexpr GLenum kGL_FLOAT = 0x1406;

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float foam;
};

static float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
static float lerpf(float a, float b, float t) { return a + (b - a) * t; }

static float hash2(int x, int y) {
    uint32_t n = static_cast<uint32_t>(x * 374761393u + y * 668265263u);
    n = (n ^ (n >> 13u)) * 1274126177u;
    return static_cast<float>((n ^ (n >> 16u)) & 0x00ffffffu) / 16777215.0f;
}

static float valueNoise(float x, float y) {
    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    float tx = x - x0;
    float ty = y - y0;
    float a = hash2(x0, y0);
    float b = hash2(x0 + 1, y0);
    float c = hash2(x0, y0 + 1);
    float d = hash2(x0 + 1, y0 + 1);
    float ux = tx * tx * (3.0f - 2.0f * tx);
    float uy = ty * ty * (3.0f - 2.0f * ty);
    return lerpf(lerpf(a, b, ux), lerpf(c, d, ux), uy);
}

static float fbm(float x, float y) {
    float sum = 0.0f;
    float amp = 1.0f;
    float freq = 0.08f;
    for (int i = 0; i < 5; ++i) {
        sum += amp * valueNoise(x * freq, y * freq);
        freq *= 2.0f;
        amp *= 0.5f;
    }
    return sum;
}

float terrainHeight(float x, float z, float time) {
    float n = fbm(x * 0.8f, z * 0.8f);
    float ridge = std::pow(std::fabs(0.5f - valueNoise(x * 1.9f, z * 1.9f)), 1.4f);
    float base = -8.0f + n * 4.0f + ridge * 0.9f;
    return base + std::sin(x * 0.22f + time * 0.7f) * 0.12f + std::cos(z * 0.19f - time * 0.5f) * 0.10f;
}

void buildTerrain(TerrainGPU &terrain) {
    constexpr int gridX = 180;
    constexpr int gridY = 140;
    std::vector<Vertex> vertices;
    vertices.reserve((gridX - 1) * (gridY - 1) * 6);

    for (int z = 0; z < gridY - 1; ++z) {
        for (int x = 0; x < gridX - 1; ++x) {
            auto sample = [&](int ix, int iz) {
                float fx = (static_cast<float>(ix) / (gridX - 1) - 0.5f) * kTerrainWidth;
                float fz = (static_cast<float>(iz) / (gridY - 1) - 0.5f) * kTerrainDepth;
                float h = terrainHeight(fx, fz, 0.0f);
                float slopeX = terrainHeight(fx + 0.3f, fz, 0.0f) - terrainHeight(fx - 0.3f, fz, 0.0f);
                float slopeZ = terrainHeight(fx, fz + 0.3f, 0.0f) - terrainHeight(fx, fz - 0.3f, 0.0f);
                float len = std::sqrt(slopeX * slopeX + 0.9f * 0.9f + slopeZ * slopeZ);
                float nx = -slopeX / len;
                float ny = 0.9f / len;
                float nz = -slopeZ / len;
                float foam = clampf((h + 8.0f) / 6.0f, 0.0f, 1.0f);
                return Vertex{fx, h, fz, nx, ny, nz, foam};
            };

            Vertex v0 = sample(x, z);
            Vertex v1 = sample(x + 1, z);
            Vertex v2 = sample(x, z + 1);
            Vertex v3 = sample(x + 1, z + 1);

            vertices.push_back(v0);
            vertices.push_back(v2);
            vertices.push_back(v1);
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
        }
    }

    terrain.count = static_cast<GLuint>(vertices.size());
    glGenVertexArrays_(1, &terrain.vao);
    glBindVertexArray_(terrain.vao);
    glGenBuffers_(1, &terrain.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, terrain.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray_(1);
    glVertexAttribPointer_(1, 3, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, nx)));
    glEnableVertexAttribArray_(2);
    glVertexAttribPointer_(2, 1, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, foam)));
    glBindVertexArray_(0);
}

void destroyTerrain(TerrainGPU &terrain) {
    terrain = {};
}
