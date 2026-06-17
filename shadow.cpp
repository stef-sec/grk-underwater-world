#include "shadow.h"

#include "shader.h"
#include "gl_loader.h"
#include "lighting.h"

#include <GL/gl.h>

static constexpr GLenum kGL_DEPTH_COMPONENT24 = 0x81A6;

Mat4 computeMoonLightMatrix(Vec3 sceneCenter, float orthoHalfSize) {
    Vec3 lightDir = vec3Normalize(kMoonLightDir);
    Vec3 lightPos = vec3Subtract(sceneCenter, vec3Scale(lightDir, 45.0f));
    Mat4 view = mat4LookAt(lightPos, sceneCenter, {0.0f, 1.0f, 0.0f});
    Mat4 proj = mat4Ortho(-orthoHalfSize, orthoHalfSize, -orthoHalfSize, orthoHalfSize, 2.0f, 90.0f);
    return mat4Multiply(proj, view);
}

void initShadow(ShadowGPU &shadow, int mapSize) {
    shadow.mapSize = mapSize;
    shadow.program = createProgramFromFiles("shaders/shadow.vert", "shaders/shadow.frag");
    shadow.uLightVP = glGetUniformLocation_(shadow.program, "uLightVP");

    glGenTextures_(1, &shadow.depthMap);
    glBindTexture_(kGL_TEXTURE_2D, shadow.depthMap);
    glTexImage2D_(kGL_TEXTURE_2D, 0, kGL_DEPTH_COMPONENT24, mapSize, mapSize, 0, kGL_DEPTH_COMPONENT, kGL_FLOAT, nullptr);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_MIN_FILTER, kGL_LINEAR);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_MAG_FILTER, kGL_LINEAR);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_WRAP_S, kGL_CLAMP_TO_BORDER);
    glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_WRAP_T, kGL_CLAMP_TO_BORDER);
    const float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv_(kGL_TEXTURE_2D, kGL_TEXTURE_BORDER_COLOR, borderColor);

    glGenFramebuffers_(1, &shadow.fbo);
    glBindFramebuffer_(kGL_FRAMEBUFFER, shadow.fbo);
    glFramebufferTexture2D_(kGL_FRAMEBUFFER, kGL_DEPTH_ATTACHMENT, kGL_TEXTURE_2D, shadow.depthMap, 0);
    glDrawBuffer_(kGL_NONE);
    glReadBuffer_(kGL_NONE);
    glBindFramebuffer_(kGL_FRAMEBUFFER, 0);
}

void beginShadowPass(ShadowGPU &shadow, const Mat4 &lightVP) {
    shadow.lightViewProj = lightVP;
    glViewport_(0, 0, shadow.mapSize, shadow.mapSize);
    glBindFramebuffer_(kGL_FRAMEBUFFER, shadow.fbo);
    glClear(kGL_DEPTH_BUFFER_BIT);
    glEnable(kGL_CULL_FACE);
    glCullFace(kGL_FRONT);
    glUseProgram_(shadow.program);
    glUniformMatrix4fv_(shadow.uLightVP, 1, 0, lightVP.m);
}

void drawShadowTerrain(const ShadowGPU &shadow, GLuint terrainVao, GLuint indexCount) {
    (void)shadow;
    glBindVertexArray_(terrainVao);
    glDrawElements_(kGL_TRIANGLES, static_cast<GLsizei>(indexCount), kGL_UNSIGNED_INT, nullptr);
    glBindVertexArray_(0);
}

void endShadowPass(int screenWidth, int screenHeight) {
    glBindFramebuffer_(kGL_FRAMEBUFFER, 0);
    glCullFace(kGL_BACK);
    glDisable(kGL_CULL_FACE);
    glViewport_(0, 0, screenWidth, screenHeight);
}

void destroyShadow(ShadowGPU &shadow) {
    if (shadow.program) glDeleteProgram_(shadow.program);
    if (shadow.fbo) glDeleteFramebuffers_(1, &shadow.fbo);
    if (shadow.depthMap) glDeleteTextures_(1, &shadow.depthMap);
    shadow = {};
}
