#pragma once

#include "gl_loader.h"
#include "math.h"

struct SubmarineGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLuint count = 0;
    bool loaded = false;
    float minY = 0.0f;
    float maxY = 0.0f;
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

void initSubmarine(SubmarineGPU &submarine);
void drawSubmarine(const SubmarineGPU &submarine, const Mat4 &viewProj, const Mat4 &model, Vec3 cameraPos, float waterLevel, float fogDensity, Vec3 moonDir, Vec3 moonColor, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity, float exposure);
void destroySubmarine(SubmarineGPU &submarine);
