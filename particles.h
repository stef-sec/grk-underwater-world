#pragma once

#include "gl_loader.h"
#include "math.h"

struct ParticlesGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLuint count = 0;
    GLint uViewProj = -1;
    GLint uCameraPos = -1;
    GLint uWaterLevel = -1;
    GLint uTime = -1;
    GLint uTint = -1;
};

void initParticles(ParticlesGPU &particles);
void drawParticles(const ParticlesGPU &particles, const Mat4 &viewProj, Vec3 cameraPos, float waterLevel, float time);
void destroyParticles(ParticlesGPU &particles);
