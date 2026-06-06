#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "water.h"

#include <vector>

using GLsizeiptr = ptrdiff_t;
using GLboolean = unsigned char;

using MyGLGenBuffersProc = void(__stdcall *)(GLsizei, GLuint *);
using MyGLBindBufferProc = void(__stdcall *)(GLenum, GLuint);
using MyGLBufferDataProc = void(__stdcall *)(GLenum, GLsizeiptr, const void *, GLenum);
using MyGLGenVertexArraysProc = void(__stdcall *)(GLsizei, GLuint *);
using MyGLBindVertexArrayProc = void(__stdcall *)(GLuint);
using MyGLEnableVertexAttribArrayProc = void(__stdcall *)(GLuint);
using MyGLVertexAttribPointerProc = void(__stdcall *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);

extern MyGLGenBuffersProc glGenBuffers_;
extern MyGLBindBufferProc glBindBuffer_;
extern MyGLBufferDataProc glBufferData_;
extern MyGLGenVertexArraysProc glGenVertexArrays_;
extern MyGLBindVertexArrayProc glBindVertexArray_;
extern MyGLEnableVertexAttribArrayProc glEnableVertexAttribArray_;
extern MyGLVertexAttribPointerProc glVertexAttribPointer_;

static constexpr GLenum kGL_ARRAY_BUFFER = 0x8892;
static constexpr GLenum kGL_STATIC_DRAW = 0x88E4;
static constexpr GLenum kGL_FLOAT = 0x1406;

struct WaterVertex {
    float px, py, pz;
};

void buildWater(WaterGPU &water) {
    std::vector<WaterVertex> vertices = {
        {-30.0f, 0.8f, -24.0f},
        {-30.0f, 0.8f, 24.0f},
        {30.0f, 0.8f, -24.0f},
        {30.0f, 0.8f, -24.0f},
        {-30.0f, 0.8f, 24.0f},
        {30.0f, 0.8f, 24.0f},
    };

    water.count = static_cast<GLuint>(vertices.size());
    glGenVertexArrays_(1, &water.vao);
    glBindVertexArray_(water.vao);
    glGenBuffers_(1, &water.vbo);
    glBindBuffer_(kGL_ARRAY_BUFFER, water.vbo);
    glBufferData_(kGL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(WaterVertex)), vertices.data(), kGL_STATIC_DRAW);
    glEnableVertexAttribArray_(0);
    glVertexAttribPointer_(0, 3, kGL_FLOAT, 0, sizeof(WaterVertex), reinterpret_cast<void *>(0));
    glBindVertexArray_(0);
}

void destroyWater(WaterGPU &water) {
    water = {};
}
