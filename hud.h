#pragma once

#include "gl_loader.h"

struct HudGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLint uScreenSize = -1;
    GLsizei vertexCount = 0;
    int cachedWidth = 0;
    int cachedHeight = 0;
    int cachedCollectedSamples = -1;
    int cachedTotalSamples = -1;
    bool cachedThirdPerson = false;
    bool cachedSpotlightEnabled = false;
    bool cachedVolumetricEnabled = false;
    float cachedVolumetricStrength = -1.0f;
};

void initHud(HudGPU &hud);
void drawHud(HudGPU &hud, int width, int height, bool thirdPerson, bool spotlightEnabled,
    bool volumetricEnabled, float volumetricStrength, int collectedSamples, int totalSamples);
void destroyHud(HudGPU &hud);
