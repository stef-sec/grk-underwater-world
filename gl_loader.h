#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include <cstddef>

#ifndef APIENTRY
#define APIENTRY __stdcall
#endif

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

constexpr GLenum kGL_VERTEX_SHADER = 0x8B31;
constexpr GLenum kGL_FRAGMENT_SHADER = 0x8B30;
constexpr GLenum kGL_COMPILE_STATUS = 0x8B81;
constexpr GLenum kGL_LINK_STATUS = 0x8B82;
constexpr GLenum kGL_INFO_LOG_LENGTH = 0x8B84;
constexpr GLenum kGL_ARRAY_BUFFER = 0x8892;
constexpr GLenum kGL_ELEMENT_ARRAY_BUFFER = 0x8893;
constexpr GLenum kGL_STATIC_DRAW = 0x88E4;
constexpr GLenum kGL_DYNAMIC_DRAW = 0x88E8;
constexpr GLenum kGL_LINES = 0x0001;
constexpr GLenum kGL_TRIANGLES = 0x0004;
constexpr GLenum kGL_PROGRAM_POINT_SIZE = 0x8642;
constexpr GLenum kGL_COLOR_BUFFER_BIT = 0x00004000;
constexpr GLenum kGL_DEPTH_BUFFER_BIT = 0x00000100;
constexpr GLenum kGL_DEPTH_TEST = 0x0B71;
constexpr GLenum kGL_BLEND = 0x0BE2;
constexpr GLenum kGL_SRC_ALPHA = 0x0302;
constexpr GLenum kGL_ONE_MINUS_SRC_ALPHA = 0x0303;
constexpr GLenum kGL_ONE = 0x0001;
constexpr GLenum kGL_UNSIGNED_INT = 0x1405;
constexpr GLenum kGL_FLOAT = 0x1406;
constexpr GLenum kGL_TEXTURE_2D = 0x0DE1;
constexpr GLenum kGL_TEXTURE_CUBE_MAP = 0x8513;
constexpr GLenum kGL_TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515;
constexpr GLenum kGL_TEXTURE0 = 0x84C0;
constexpr GLenum kGL_TEXTURE_MIN_FILTER = 0x2801;
constexpr GLenum kGL_TEXTURE_MAG_FILTER = 0x2800;
constexpr GLenum kGL_TEXTURE_WRAP_S = 0x2802;
constexpr GLenum kGL_TEXTURE_WRAP_T = 0x2803;
constexpr GLenum kGL_LINEAR = 0x2601;
constexpr GLenum kGL_CLAMP_TO_EDGE = 0x812F;
constexpr GLenum kGL_CLAMP_TO_BORDER = 0x812D;
constexpr GLenum kGL_TEXTURE_BORDER_COLOR = 0x1004;
constexpr GLenum kGL_DEPTH_COMPONENT = 0x1902;
constexpr GLenum kGL_FRAMEBUFFER = 0x8D40;
constexpr GLenum kGL_DEPTH_ATTACHMENT = 0x8D00;
constexpr GLenum kGL_NONE = 0;
constexpr GLenum kGL_FRAMEBUFFER_COMPLETE = 0x8CD5;
constexpr GLenum kGL_FRONT = 0x0404;
constexpr GLenum kGL_BACK = 0x0405;
constexpr GLenum kGL_CW = 0x0900;
constexpr GLenum kGL_CCW = 0x0901;
constexpr GLenum kGL_LESS = 0x0201;
constexpr GLenum kGL_LEQUAL = 0x0203;
constexpr GLenum kGL_CULL_FACE = 0x0B44;

using GLCreateShaderFn = GLuint(APIENTRY *)(GLenum);
using GLShaderSourceFn = void(APIENTRY *)(GLuint, GLsizei, const GLchar *const *, const GLint *);
using GLCompileShaderFn = void(APIENTRY *)(GLuint);
using GLGetShaderivFn = void(APIENTRY *)(GLuint, GLenum, GLint *);
using GLGetShaderInfoLogFn = void(APIENTRY *)(GLuint, GLsizei, GLsizei *, GLchar *);
using GLCreateProgramFn = GLuint(APIENTRY *)();
using GLAttachShaderFn = void(APIENTRY *)(GLuint, GLuint);
using GLLinkProgramFn = void(APIENTRY *)(GLuint);
using GLGetProgramivFn = void(APIENTRY *)(GLuint, GLenum, GLint *);
using GLGetProgramInfoLogFn = void(APIENTRY *)(GLuint, GLsizei, GLsizei *, GLchar *);
using GLUseProgramFn = void(APIENTRY *)(GLuint);
using GLDeleteShaderFn = void(APIENTRY *)(GLuint);
using GLDeleteProgramFn = void(APIENTRY *)(GLuint);
using GLGenBuffersFn = void(APIENTRY *)(GLsizei, GLuint *);
using GLBindBufferFn = void(APIENTRY *)(GLenum, GLuint);
using GLBufferDataFn = void(APIENTRY *)(GLenum, GLsizeiptr, const void *, GLenum);
using GLGenVertexArraysFn = void(APIENTRY *)(GLsizei, GLuint *);
using GLBindVertexArrayFn = void(APIENTRY *)(GLuint);
using GLEnableVertexAttribArrayFn = void(APIENTRY *)(GLuint);
using GLVertexAttribPointerFn = void(APIENTRY *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
using GLGetUniformLocationFn = GLint(APIENTRY *)(GLuint, const GLchar *);
using GLUniform1fFn = void(APIENTRY *)(GLint, GLfloat);
using GLUniform1iFn = void(APIENTRY *)(GLint, GLint);
using GLUniform3fFn = void(APIENTRY *)(GLint, GLfloat, GLfloat, GLfloat);
using GLUniformMatrix4fvFn = void(APIENTRY *)(GLint, GLsizei, GLboolean, const GLfloat *);
using GLDrawElementsFn = void(APIENTRY *)(GLenum, GLsizei, GLenum, const void *);
using GLDeleteBuffersFn = void(APIENTRY *)(GLsizei, const GLuint *);
using GLDeleteVertexArraysFn = void(APIENTRY *)(GLsizei, const GLuint *);
using GLGenTexturesFn = void(APIENTRY *)(GLsizei, GLuint *);
using GLBindTextureFn = void(APIENTRY *)(GLenum, GLuint);
using GLTexImage2DFn = void(APIENTRY *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
using GLTexParameteriFn = void(APIENTRY *)(GLenum, GLenum, GLint);
using GLTexParameterfvFn = void(APIENTRY *)(GLenum, GLenum, const GLfloat *);
using GLActiveTextureFn = void(APIENTRY *)(GLenum);
using GLGenFramebuffersFn = void(APIENTRY *)(GLsizei, GLuint *);
using GLBindFramebufferFn = void(APIENTRY *)(GLenum, GLuint);
using GLFramebufferTexture2DFn = void(APIENTRY *)(GLenum, GLenum, GLenum, GLuint, GLint);
using GLDrawBufferFn = void(APIENTRY *)(GLenum);
using GLReadBufferFn = void(APIENTRY *)(GLenum);
using GLCheckFramebufferStatusFn = GLenum(APIENTRY *)(GLenum);
using GLDeleteFramebuffersFn = void(APIENTRY *)(GLsizei, const GLuint *);
using GLDeleteTexturesFn = void(APIENTRY *)(GLsizei, const GLuint *);
using GLViewportFn = void(APIENTRY *)(GLint, GLint, GLsizei, GLsizei);

extern GLCreateShaderFn glCreateShader_;
extern GLShaderSourceFn glShaderSource_;
extern GLCompileShaderFn glCompileShader_;
extern GLGetShaderivFn glGetShaderiv_;
extern GLGetShaderInfoLogFn glGetShaderInfoLog_;
extern GLCreateProgramFn glCreateProgram_;
extern GLAttachShaderFn glAttachShader_;
extern GLLinkProgramFn glLinkProgram_;
extern GLGetProgramivFn glGetProgramiv_;
extern GLGetProgramInfoLogFn glGetProgramInfoLog_;
extern GLUseProgramFn glUseProgram_;
extern GLDeleteShaderFn glDeleteShader_;
extern GLDeleteProgramFn glDeleteProgram_;
extern GLGenBuffersFn glGenBuffers_;
extern GLBindBufferFn glBindBuffer_;
extern GLBufferDataFn glBufferData_;
extern GLGenVertexArraysFn glGenVertexArrays_;
extern GLBindVertexArrayFn glBindVertexArray_;
extern GLEnableVertexAttribArrayFn glEnableVertexAttribArray_;
extern GLVertexAttribPointerFn glVertexAttribPointer_;
extern GLGetUniformLocationFn glGetUniformLocation_;
extern GLUniform1fFn glUniform1f_;
extern GLUniform1iFn glUniform1i_;
extern GLUniform3fFn glUniform3f_;
extern GLUniformMatrix4fvFn glUniformMatrix4fv_;
extern GLDrawElementsFn glDrawElements_;
extern GLDeleteBuffersFn glDeleteBuffers_;
extern GLDeleteVertexArraysFn glDeleteVertexArrays_;
extern GLGenTexturesFn glGenTextures_;
extern GLBindTextureFn glBindTexture_;
extern GLTexImage2DFn glTexImage2D_;
extern GLTexParameteriFn glTexParameteri_;
extern GLTexParameterfvFn glTexParameterfv_;
extern GLActiveTextureFn glActiveTexture_;
extern GLGenFramebuffersFn glGenFramebuffers_;
extern GLBindFramebufferFn glBindFramebuffer_;
extern GLFramebufferTexture2DFn glFramebufferTexture2D_;
extern GLDrawBufferFn glDrawBuffer_;
extern GLReadBufferFn glReadBuffer_;
extern GLCheckFramebufferStatusFn glCheckFramebufferStatus_;
extern GLDeleteFramebuffersFn glDeleteFramebuffers_;
extern GLDeleteTexturesFn glDeleteTextures_;
extern GLViewportFn glViewport_;

bool loadGLFunctions();
GLuint compileShader(GLenum type, const char *source);
GLuint createProgram(const char *vs, const char *fs);
