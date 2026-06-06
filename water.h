#pragma once

#include "terrain.h"

struct WaterGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint count = 0;
};

void buildWater(WaterGPU &water);
void destroyWater(WaterGPU &water);
