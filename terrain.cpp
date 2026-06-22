#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "terrain.h"
#include "gl_loader.h"

#include <cmath>
#include <cstdint>
#include <vector>

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float tx, ty, tz;
    float bx, by, bz;
    float foam;
    float slope;
};

static float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

static float smoothstep01(float a, float b, float x) {
    float t = clampf((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

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

static float terrainHeightBase(float x, float z) {
    float n = fbm(x * 0.75f, z * 0.75f);
    float ridge = std::pow(std::fabs(0.5f - valueNoise(x * 1.7f, z * 1.7f)), 1.45f);
    float basinSource = 0.65f - valueNoise(x * 0.35f, z * 0.35f);
    float basin = -0.85f * std::pow(basinSource > 0.0f ? basinSource : 0.0f, 2.2f);
    return -8.0f + n * 3.7f + ridge * 1.0f + basin * 2.4f;
}

float terrainHeight(float x, float z, float time) {
    return terrainHeightBase(x, z) + std::sin(x * 0.22f + time * 0.7f) * 0.12f + std::cos(z * 0.19f - time * 0.5f) * 0.10f;
}

void buildTerrain(TerrainGPU &terrain) {
    constexpr int gridX = 180;
    constexpr int gridZ = 140;
    const float cellW = kTerrainWidth / static_cast<float>(gridX - 1);
    const float cellD = kTerrainDepth / static_cast<float>(gridZ - 1);

    std::vector<float> heights(static_cast<size_t>(gridX * gridZ));
    for (int z = 0; z < gridZ; ++z) {
        for (int x = 0; x < gridX; ++x) {
            float fx = (static_cast<float>(x) / static_cast<float>(gridX - 1) - 0.5f) * kTerrainWidth;
            float fz = (static_cast<float>(z) / static_cast<float>(gridZ - 1) - 0.5f) * kTerrainDepth;
            heights[static_cast<size_t>(z * gridX + x)] = terrainHeightBase(fx, fz);
        }
    }

    auto sampleHeight = [&](int x, int z) -> float {
        x = x < 0 ? 0 : (x >= gridX ? gridX - 1 : x);
        z = z < 0 ? 0 : (z >= gridZ ? gridZ - 1 : z);
        return heights[static_cast<size_t>(z * gridX + x)];
    };

    std::vector<Vertex> vertices(static_cast<size_t>(gridX * gridZ));
    for (int z = 0; z < gridZ; ++z) {
        for (int x = 0; x < gridX; ++x) {
            float fx = (static_cast<float>(x) / static_cast<float>(gridX - 1) - 0.5f) * kTerrainWidth;
            float fz = (static_cast<float>(z) / static_cast<float>(gridZ - 1) - 0.5f) * kTerrainDepth;
            float h = heights[static_cast<size_t>(z * gridX + x)];

            float slopeX = (sampleHeight(x + 1, z) - sampleHeight(x - 1, z)) / (2.0f * cellW);
            float slopeZ = (sampleHeight(x, z + 1) - sampleHeight(x, z - 1)) / (2.0f * cellD);

            float pxm = fx - cellW;
            float pxp = fx + cellW;
            float pzm = fz - cellD;
            float pzp = fz + cellD;
            float hxm = sampleHeight(x - 1, z);
            float hxp = sampleHeight(x + 1, z);
            float hzm = sampleHeight(x, z - 1);
            float hzp = sampleHeight(x, z + 1);

            float tx = pxp - pxm;
            float ty = hxp - hxm;
            float tz = 0.0f;
            float tLen = std::sqrt(tx * tx + ty * ty + tz * tz);
            float tanX = 0.0f, tanY = 0.0f, tanZ = 0.0f;
            if (tLen > 0.00001f) {
                tanX = tx / tLen;
                tanY = ty / tLen;
                tanZ = tz / tLen;
            } else {
                tanX = 1.0f;
            }

            float bx = 0.0f;
            float by = hzp - hzm;
            float bz = pzp - pzm;
            float bLen = std::sqrt(bx * bx + by * by + bz * bz);
            float bitX = 0.0f, bitY = 0.0f, bitZ = 0.0f;
            if (bLen > 0.00001f) {
                bitX = bx / bLen;
                bitY = by / bLen;
                bitZ = bz / bLen;
            } else {
                bitZ = 1.0f;
            }

            float nx = -slopeX;
            float ny = 1.0f;
            float nz = -slopeZ;
            float len = std::sqrt(nx * nx + ny * ny + nz * nz);
            if (len > 0.00001f) {
                nx /= len;
                ny /= len;
                nz /= len;
            } else {
                nx = 0.0f;
                ny = 1.0f;
                nz = 0.0f;
            }

            float foam = smoothstep01(-6.0f, -2.0f, h);
            float slope = clampf(std::sqrt(nx * nx + nz * nz), 0.0f, 1.0f);

            vertices[static_cast<size_t>(z * gridX + x)] = Vertex{
                fx, h, fz, nx, ny, nz, tanX, tanY, tanZ, bitX, bitY, bitZ, foam, slope};
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>((gridX - 1) * (gridZ - 1) * 6));
    for (int z = 0; z < gridZ - 1; ++z) {
        for (int x = 0; x < gridX - 1; ++x) {
            uint32_t i0 = static_cast<uint32_t>(z * gridX + x);
            uint32_t i1 = i0 + 1u;
            uint32_t i2 = i0 + static_cast<uint32_t>(gridX);
            uint32_t i3 = i2 + 1u;
            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    terrain.indexCount = static_cast<GLuint>(indices.size());

    glGenVertexArrays_(1, &terrain.vao);
    glBindVertexArray_(terrain.vao);

    glGenBuffers_(1, &terrain.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, terrain.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), kGL_STATIC_DRAW);

    glGenBuffers_(1, &terrain.ebo);
    glBindBuffer_(kGL_ELEMENT_ARRAY_BUFFER, terrain.ebo);
    glBufferData_(kGL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)), indices.data(), kGL_STATIC_DRAW);

    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray_(1);
    glVertexAttribPointer_(1, 3, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, nx)));
    glEnableVertexAttribArray_(2);
    glVertexAttribPointer_(2, 3, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, tx)));
    glEnableVertexAttribArray_(3);
    glVertexAttribPointer_(3, 3, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, bx)));
    glEnableVertexAttribArray_(4);
    glVertexAttribPointer_(4, 1, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, foam)));
    glEnableVertexAttribArray_(5);
    glVertexAttribPointer_(5, 1, kGL_FLOAT, 0, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, slope)));

    glBindVertexArray_(0);
}

void destroyTerrain(TerrainGPU &terrain) {
    if (terrain.ebo) glDeleteBuffers_(1, &terrain.ebo);
    if (terrain.vbo) glDeleteBuffers_(1, &terrain.vbo);
    if (terrain.vao) glDeleteVertexArrays_(1, &terrain.vao);
    terrain = {};
}
