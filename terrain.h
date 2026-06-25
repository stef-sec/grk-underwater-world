#pragma once

#include "gl_loader.h"

struct TerrainGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint indexCount = 0;
};

constexpr float kTerrainWidth = 96.0f;
constexpr float kTerrainDepth = 76.0f;

float terrainHeight(float x, float z, float time);
void buildTerrain(TerrainGPU &terrain);
void destroyTerrain(TerrainGPU &terrain);
