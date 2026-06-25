#include "volumetric.h"

#include "gl_loader.h"
#include "shader.h"

#include <GL/gl.h>

static const float kFullscreenTriangle[] = {
    -1.0f, -1.0f,
     3.0f, -1.0f,
    -1.0f,  3.0f,
};

void initVolumetric(VolumetricGPU &vol) {
    vol.program = createProgramFromFiles("shaders/volumetric.vert", "shaders/volumetric.frag");
    vol.uInvViewProj = glGetUniformLocation_(vol.program, "uInvViewProj");
    vol.uCameraPos = glGetUniformLocation_(vol.program, "uCameraPos");
    vol.uWaterLevel = glGetUniformLocation_(vol.program, "uWaterLevel");
    vol.uFogDensity = glGetUniformLocation_(vol.program, "uFogDensity");
    vol.uStrength = glGetUniformLocation_(vol.program, "uStrength");
    vol.uTime = glGetUniformLocation_(vol.program, "uTime");
    vol.uMoonDir = glGetUniformLocation_(vol.program, "uMoonDir");
    vol.uMoonColor = glGetUniformLocation_(vol.program, "uMoonColor");
    vol.uSpotPos = glGetUniformLocation_(vol.program, "uSpotPos");
    vol.uSpotDir = glGetUniformLocation_(vol.program, "uSpotDir");
    vol.uSpotColor = glGetUniformLocation_(vol.program, "uSpotColor");
    vol.uSpotInner = glGetUniformLocation_(vol.program, "uSpotInner");
    vol.uSpotOuter = glGetUniformLocation_(vol.program, "uSpotOuter");
    vol.uSpotIntensity = glGetUniformLocation_(vol.program, "uSpotIntensity");
    vol.uSceneDepth = glGetUniformLocation_(vol.program, "uSceneDepth");

    glGenVertexArrays_(1, &vol.vao);
    glBindVertexArray_(vol.vao);
    glGenBuffers_(1, &vol.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, vol.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, sizeof(kFullscreenTriangle), kFullscreenTriangle, kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 2, kGL_FLOAT, 0, 2 * sizeof(float), reinterpret_cast<void *>(0));
    glBindVertexArray_(0);
}

void captureVolumetricDepth(VolumetricGPU &vol, int width, int height) {
    if (width <= 0 || height <= 0) return;

    const bool needAllocate = !vol.depthTexture || vol.depthWidth != width || vol.depthHeight != height;
    if (!vol.depthTexture) {
        glGenTextures_(1, &vol.depthTexture);
        glBindTexture_(kGL_TEXTURE_2D, vol.depthTexture);
        glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_MIN_FILTER, kGL_LINEAR);
        glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_MAG_FILTER, kGL_LINEAR);
        glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_WRAP_S, kGL_CLAMP_TO_EDGE);
        glTexParameteri_(kGL_TEXTURE_2D, kGL_TEXTURE_WRAP_T, kGL_CLAMP_TO_EDGE);
    } else {
        glBindTexture_(kGL_TEXTURE_2D, vol.depthTexture);
    }

    if (needAllocate) {
        glTexImage2D_(kGL_TEXTURE_2D, 0, kGL_DEPTH_COMPONENT, width, height, 0, kGL_DEPTH_COMPONENT, kGL_FLOAT, nullptr);
    }
    glCopyTexSubImage2D(kGL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
    vol.depthWidth = width;
    vol.depthHeight = height;
}

void drawVolumetric(const VolumetricGPU &vol, const Mat4 &invViewProj, Vec3 cameraPos, float waterLevel, float fogDensity, float strength, float time, Vec3 moonDir, Vec3 moonColor, Vec3 spotPos, Vec3 spotDir, Vec3 spotColor, float spotInner, float spotOuter, float spotIntensity) {
    if (!vol.program || strength <= 0.0001f) return;

    glDisable(kGL_DEPTH_TEST);
    glDepthMask(0);
    glEnable(kGL_BLEND);
    glBlendFunc(kGL_SRC_ALPHA, kGL_ONE);

    glUseProgram_(vol.program);
    glUniformMatrix4fv_(vol.uInvViewProj, 1, 0, invViewProj.m);
    glUniform3f_(vol.uCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);
    glUniform1f_(vol.uWaterLevel, waterLevel);
    glUniform1f_(vol.uFogDensity, fogDensity);
    glUniform1f_(vol.uStrength, strength);
    glUniform1f_(vol.uTime, time);
    glUniform3f_(vol.uMoonDir, moonDir.x, moonDir.y, moonDir.z);
    glUniform3f_(vol.uMoonColor, moonColor.x, moonColor.y, moonColor.z);
    glUniform3f_(vol.uSpotPos, spotPos.x, spotPos.y, spotPos.z);
    glUniform3f_(vol.uSpotDir, spotDir.x, spotDir.y, spotDir.z);
    glUniform3f_(vol.uSpotColor, spotColor.x, spotColor.y, spotColor.z);
    glUniform1f_(vol.uSpotInner, spotInner);
    glUniform1f_(vol.uSpotOuter, spotOuter);
    glUniform1f_(vol.uSpotIntensity, spotIntensity);
    glActiveTexture_(kGL_TEXTURE0 + 1);
    glBindTexture_(kGL_TEXTURE_2D, vol.depthTexture);
    glUniform1i_(vol.uSceneDepth, 1);
    glBindVertexArray_(vol.vao);
    glDrawArrays(kGL_TRIANGLES, 0, 3);
    glBindVertexArray_(0);
    glUseProgram_(0);
    glActiveTexture_(kGL_TEXTURE0);

    glBlendFunc(kGL_SRC_ALPHA, kGL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(1);
    glEnable(kGL_DEPTH_TEST);
}

void destroyVolumetric(VolumetricGPU &vol) {
    if (vol.program) glDeleteProgram_(vol.program);
    if (vol.vbo) glDeleteBuffers_(1, &vol.vbo);
    if (vol.vao) glDeleteVertexArrays_(1, &vol.vao);
    if (vol.depthTexture) glDeleteTextures_(1, &vol.depthTexture);
    vol = {};
}
