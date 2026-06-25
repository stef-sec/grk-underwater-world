#pragma once

#include "gl_loader.h"
#include "math.h"

struct VolumetricGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLint uInvViewProj = -1;
    GLint uCameraPos = -1;
    GLint uWaterLevel = -1;
    GLint uFogDensity = -1;
    GLint uStrength = -1;
    GLint uTime = -1;
    GLint uMoonDir = -1;
    GLint uMoonColor = -1;
    GLint uSpotPos = -1;
    GLint uSpotDir = -1;
    GLint uSpotColor = -1;
    GLint uSpotInner = -1;
    GLint uSpotOuter = -1;
    GLint uSpotIntensity = -1;
    GLint uSceneDepth = -1;
    GLuint depthTexture = 0;
    int depthWidth = 0;
    int depthHeight = 0;
};

void initVolumetric(VolumetricGPU &vol);
void captureVolumetricDepth(VolumetricGPU &vol, int width, int height);
void drawVolumetric(const VolumetricGPU &vol, const Mat4 &invViewProj, Vec3 cameraPos, float waterLevel, float fogDensity, float strength, float time, Vec3 moonDir, Vec3 moonColor, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity);
void destroyVolumetric(VolumetricGPU &vol);
