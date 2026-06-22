#pragma once

#include "gl_loader.h"

#include <cstdint>

struct SkyboxGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint cubemap = 0;
    GLuint program = 0;
    GLint uView = -1;
    GLint uProj = -1;
    GLint uCubemap = -1;
};

void initSkybox(SkyboxGPU &skybox);
void drawSkybox(const SkyboxGPU &skybox, const float viewNoTranslation[16], const float proj[16]);
void destroySkybox(SkyboxGPU &skybox);
