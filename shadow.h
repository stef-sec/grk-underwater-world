#pragma once

#include "gl_loader.h"
#include "math.h"

struct ShadowGPU {
    GLuint fbo = 0;
    GLuint depthMap = 0;
    GLuint program = 0;
    GLint uLightVP = -1;
    int mapSize = 2048;
    Mat4 lightViewProj{};
};

void initShadow(ShadowGPU &shadow, int mapSize);
Mat4 computeMoonLightMatrix(Vec3 sceneCenter, float orthoHalfSize);
void beginShadowPass(ShadowGPU &shadow, const Mat4 &lightVP);
void drawShadowTerrain(const ShadowGPU &shadow, GLuint terrainVao, GLuint indexCount);
void endShadowPass(int screenWidth, int screenHeight);
void destroyShadow(ShadowGPU &shadow);
