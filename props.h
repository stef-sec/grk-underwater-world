#pragma once

#include "gl_loader.h"
#include "math.h"

#include <vector>

struct SeaweedInstance {
    float x, y, z;
    float scale;
    float rotY;
    bool collected = false;
};

struct SeaweedGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLuint count = 0;
    float minY = 0.0f;
    float spanX = 1.0f;
    float spanZ = 1.0f;
    bool loaded = false;
    std::vector<SeaweedInstance> instances;
    GLint uViewProj = -1;
    GLint uModel = -1;
    GLint uTime = -1;
    GLint uWaterLevel = -1;
    GLint uFogDensity = -1;
    GLint uLightDir = -1;
    GLint uDeepColor = -1;
    GLint uColor = -1;
    GLint uSpotPos = -1;
    GLint uSpotDir = -1;
    GLint uSpotColor = -1;
    GLint uSpotInner = -1;
    GLint uSpotOuter = -1;
    GLint uSpotIntensity = -1;
    GLint uExposure = -1;
};

struct RockInstance {
    float x, y, z;
    float scale;
    float rotY;
    float colorBias;
};

struct RockGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLuint count = 0;
    bool loaded = false;
    std::vector<RockInstance> instances;
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
};

void initSeaweed(SeaweedGPU &seaweed, float waterLevel);
void drawSeaweed(const SeaweedGPU &seaweed, const Mat4 &viewProj, float time, float waterLevel, float fogDensity, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity, float exposure, int highlightedIndex);
void destroySeaweed(SeaweedGPU &seaweed);

void initRocks(RockGPU &rocks, float waterLevel);
void drawRocks(const RockGPU &rocks, const Mat4 &viewProj, Vec3 cameraPos, float waterLevel, float fogDensity, Vec3 moonDir, Vec3 moonColor, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity, float exposure);
void destroyRocks(RockGPU &rocks);
