#pragma once

#include <cstdint>

using GLuint = unsigned int;

struct TerrainGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint count = 0;
};

constexpr float kTerrainWidth = 60.0f;
constexpr float kTerrainDepth = 48.0f;

float terrainHeight(float x, float z, float time);
void buildTerrain(TerrainGPU &terrain);
void destroyTerrain(TerrainGPU &terrain);
