#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "particles.h"

#include "shader.h"
#include "terrain.h"

#include <cstdint>
#include <cstddef>
#include <vector>

struct ParticleVertex {
    float x, y, z;
    float seed;
    float size;
};

static float hash01(uint32_t n) {
    n ^= n >> 16;
    n *= 0x7feb352du;
    n ^= n >> 15;
    n *= 0x846ca68bu;
    n ^= n >> 16;
    return static_cast<float>(n & 0x00ffffffu) / 16777215.0f;
}

void initParticles(ParticlesGPU &particles) {
    particles.program = createProgramFromFiles("shaders/particles.vert", "shaders/particles.frag");
    particles.uViewProj = glGetUniformLocation_(particles.program, "uViewProj");
    particles.uCameraPos = glGetUniformLocation_(particles.program, "uCameraPos");
    particles.uWaterLevel = glGetUniformLocation_(particles.program, "uWaterLevel");
    particles.uTime = glGetUniformLocation_(particles.program, "uTime");
    particles.uTint = glGetUniformLocation_(particles.program, "uTint");

    constexpr int kCount = 360;
    std::vector<ParticleVertex> vertices;
    vertices.reserve(kCount);
    for (int i = 0; i < kCount; ++i) {
        const float rx = hash01(static_cast<uint32_t>(i * 37 + 11));
        const float ry = hash01(static_cast<uint32_t>(i * 53 + 19));
        const float rz = hash01(static_cast<uint32_t>(i * 97 + 23));
        const float rs = hash01(static_cast<uint32_t>(i * 131 + 29));
        const float x = (rx - 0.5f) * kTerrainWidth * 0.92f;
        const float z = (rz - 0.5f) * kTerrainDepth * 0.92f;
        const float y = -17.0f + ry * 17.5f;
        const float size = 2.0f + rs * 4.5f;
        vertices.push_back(ParticleVertex{x, y, z, hash01(static_cast<uint32_t>(i * 173 + 31)), size});
    }

    particles.count = static_cast<GLuint>(vertices.size());
    glGenVertexArrays_(1, &particles.vao);
    glBindVertexArray_(particles.vao);
    glGenBuffers_(1, &particles.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, particles.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(ParticleVertex)), vertices.data(), kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(ParticleVertex), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray_(1);
    glVertexAttribPointer_(1, 1, kGL_FLOAT, 0, sizeof(ParticleVertex), reinterpret_cast<void *>(offsetof(ParticleVertex, seed)));
    glEnableVertexAttribArray_(2);
    glVertexAttribPointer_(2, 1, kGL_FLOAT, 0, sizeof(ParticleVertex), reinterpret_cast<void *>(offsetof(ParticleVertex, size)));
    glBindVertexArray_(0);
}

void drawParticles(const ParticlesGPU &particles, const Mat4 &viewProj, Vec3 cameraPos, float waterLevel, float time) {
    if (!particles.program || particles.count == 0) return;

    glEnable(kGL_PROGRAM_POINT_SIZE);
    glEnable(kGL_BLEND);
    glBlendFunc(kGL_SRC_ALPHA, kGL_ONE);
    glDepthMask(0);

    glUseProgram_(particles.program);
    glUniformMatrix4fv_(particles.uViewProj, 1, 0, viewProj.m);
    glUniform3f_(particles.uCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform1f_(particles.uWaterLevel, waterLevel);
    glUniform1f_(particles.uTime, time);
    glUniform3f_(particles.uTint, 0.45f, 0.78f, 1.0f);
    glBindVertexArray_(particles.vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(particles.count));
    glBindVertexArray_(0);
    glUseProgram_(0);

    glDepthMask(1);
    glBlendFunc(kGL_SRC_ALPHA, kGL_ONE_MINUS_SRC_ALPHA);
    glDisable(kGL_BLEND);
    glDisable(kGL_PROGRAM_POINT_SIZE);
}

void destroyParticles(ParticlesGPU &particles) {
    if (particles.program) glDeleteProgram_(particles.program);
    if (particles.vbo) glDeleteBuffers_(1, &particles.vbo);
    if (particles.vao) glDeleteVertexArrays_(1, &particles.vao);
    particles = {};
}
