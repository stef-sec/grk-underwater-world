#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "gl_loader.h"

#include <cstddef>
#include <string>

GLCreateShaderFn glCreateShader_ = nullptr;
GLShaderSourceFn glShaderSource_ = nullptr;
GLCompileShaderFn glCompileShader_ = nullptr;
GLGetShaderivFn glGetShaderiv_ = nullptr;
GLGetShaderInfoLogFn glGetShaderInfoLog_ = nullptr;
GLCreateProgramFn glCreateProgram_ = nullptr;
GLAttachShaderFn glAttachShader_ = nullptr;
GLLinkProgramFn glLinkProgram_ = nullptr;
GLGetProgramivFn glGetProgramiv_ = nullptr;
GLGetProgramInfoLogFn glGetProgramInfoLog_ = nullptr;
GLUseProgramFn glUseProgram_ = nullptr;
GLDeleteShaderFn glDeleteShader_ = nullptr;
GLDeleteProgramFn glDeleteProgram_ = nullptr;
GLGenBuffersFn glGenBuffers_ = nullptr;
GLBindBufferFn glBindBuffer_ = nullptr;
GLBufferDataFn glBufferData_ = nullptr;
GLGenVertexArraysFn glGenVertexArrays_ = nullptr;
GLBindVertexArrayFn glBindVertexArray_ = nullptr;
GLEnableVertexAttribArrayFn glEnableVertexAttribArray_ = nullptr;
GLVertexAttribPointerFn glVertexAttribPointer_ = nullptr;
GLGetUniformLocationFn glGetUniformLocation_ = nullptr;
GLUniform1fFn glUniform1f_ = nullptr;
GLUniform1iFn glUniform1i_ = nullptr;
GLUniform3fFn glUniform3f_ = nullptr;
GLUniformMatrix4fvFn glUniformMatrix4fv_ = nullptr;
GLDrawElementsFn glDrawElements_ = nullptr;
GLDeleteBuffersFn glDeleteBuffers_ = nullptr;
GLDeleteVertexArraysFn glDeleteVertexArrays_ = nullptr;
GLGenTexturesFn glGenTextures_ = nullptr;
GLBindTextureFn glBindTexture_ = nullptr;
GLTexImage2DFn glTexImage2D_ = nullptr;
GLTexParameteriFn glTexParameteri_ = nullptr;
GLTexParameterfvFn glTexParameterfv_ = nullptr;
GLActiveTextureFn glActiveTexture_ = nullptr;
GLGenFramebuffersFn glGenFramebuffers_ = nullptr;
GLBindFramebufferFn glBindFramebuffer_ = nullptr;
GLFramebufferTexture2DFn glFramebufferTexture2D_ = nullptr;
GLDrawBufferFn glDrawBuffer_ = nullptr;
GLReadBufferFn glReadBuffer_ = nullptr;
GLCheckFramebufferStatusFn glCheckFramebufferStatus_ = nullptr;
GLDeleteFramebuffersFn glDeleteFramebuffers_ = nullptr;
GLDeleteTexturesFn glDeleteTextures_ = nullptr;
GLViewportFn glViewport_ = nullptr;

static void *loadProc(const char *name) {
    void *p = reinterpret_cast<void *>(wglGetProcAddress(name));
    if (!p) {
        HMODULE mod = GetModuleHandleA("opengl32.dll");
        p = reinterpret_cast<void *>(GetProcAddress(mod, name));
    }
    return p;
}

bool loadGLFunctions() {
    glCreateShader_ = reinterpret_cast<GLCreateShaderFn>(loadProc("glCreateShader"));
    glShaderSource_ = reinterpret_cast<GLShaderSourceFn>(loadProc("glShaderSource"));
    glCompileShader_ = reinterpret_cast<GLCompileShaderFn>(loadProc("glCompileShader"));
    glGetShaderiv_ = reinterpret_cast<GLGetShaderivFn>(loadProc("glGetShaderiv"));
    glGetShaderInfoLog_ = reinterpret_cast<GLGetShaderInfoLogFn>(loadProc("glGetShaderInfoLog"));
    glCreateProgram_ = reinterpret_cast<GLCreateProgramFn>(loadProc("glCreateProgram"));
    glAttachShader_ = reinterpret_cast<GLAttachShaderFn>(loadProc("glAttachShader"));
    glLinkProgram_ = reinterpret_cast<GLLinkProgramFn>(loadProc("glLinkProgram"));
    glGetProgramiv_ = reinterpret_cast<GLGetProgramivFn>(loadProc("glGetProgramiv"));
    glGetProgramInfoLog_ = reinterpret_cast<GLGetProgramInfoLogFn>(loadProc("glGetProgramInfoLog"));
    glUseProgram_ = reinterpret_cast<GLUseProgramFn>(loadProc("glUseProgram"));
    glDeleteShader_ = reinterpret_cast<GLDeleteShaderFn>(loadProc("glDeleteShader"));
    glDeleteProgram_ = reinterpret_cast<GLDeleteProgramFn>(loadProc("glDeleteProgram"));
    glGenBuffers_ = reinterpret_cast<GLGenBuffersFn>(loadProc("glGenBuffers"));
    glBindBuffer_ = reinterpret_cast<GLBindBufferFn>(loadProc("glBindBuffer"));
    glBufferData_ = reinterpret_cast<GLBufferDataFn>(loadProc("glBufferData"));
    glGenVertexArrays_ = reinterpret_cast<GLGenVertexArraysFn>(loadProc("glGenVertexArrays"));
    glBindVertexArray_ = reinterpret_cast<GLBindVertexArrayFn>(loadProc("glBindVertexArray"));
    glEnableVertexAttribArray_ = reinterpret_cast<GLEnableVertexAttribArrayFn>(loadProc("glEnableVertexAttribArray"));
    glVertexAttribPointer_ = reinterpret_cast<GLVertexAttribPointerFn>(loadProc("glVertexAttribPointer"));
    glGetUniformLocation_ = reinterpret_cast<GLGetUniformLocationFn>(loadProc("glGetUniformLocation"));
    glUniform1f_ = reinterpret_cast<GLUniform1fFn>(loadProc("glUniform1f"));
    glUniform1i_ = reinterpret_cast<GLUniform1iFn>(loadProc("glUniform1i"));
    glUniform3f_ = reinterpret_cast<GLUniform3fFn>(loadProc("glUniform3f"));
    glUniformMatrix4fv_ = reinterpret_cast<GLUniformMatrix4fvFn>(loadProc("glUniformMatrix4fv"));
    glDrawElements_ = reinterpret_cast<GLDrawElementsFn>(loadProc("glDrawElements"));
    glDeleteBuffers_ = reinterpret_cast<GLDeleteBuffersFn>(loadProc("glDeleteBuffers"));
    glDeleteVertexArrays_ = reinterpret_cast<GLDeleteVertexArraysFn>(loadProc("glDeleteVertexArrays"));
    glGenTextures_ = reinterpret_cast<GLGenTexturesFn>(loadProc("glGenTextures"));
    glBindTexture_ = reinterpret_cast<GLBindTextureFn>(loadProc("glBindTexture"));
    glTexImage2D_ = reinterpret_cast<GLTexImage2DFn>(loadProc("glTexImage2D"));
    glTexParameteri_ = reinterpret_cast<GLTexParameteriFn>(loadProc("glTexParameteri"));
    glTexParameterfv_ = reinterpret_cast<GLTexParameterfvFn>(loadProc("glTexParameterfv"));
    glActiveTexture_ = reinterpret_cast<GLActiveTextureFn>(loadProc("glActiveTexture"));
    glGenFramebuffers_ = reinterpret_cast<GLGenFramebuffersFn>(loadProc("glGenFramebuffers"));
    glBindFramebuffer_ = reinterpret_cast<GLBindFramebufferFn>(loadProc("glBindFramebuffer"));
    glFramebufferTexture2D_ = reinterpret_cast<GLFramebufferTexture2DFn>(loadProc("glFramebufferTexture2D"));
    glDrawBuffer_ = reinterpret_cast<GLDrawBufferFn>(loadProc("glDrawBuffer"));
    glReadBuffer_ = reinterpret_cast<GLReadBufferFn>(loadProc("glReadBuffer"));
    glCheckFramebufferStatus_ = reinterpret_cast<GLCheckFramebufferStatusFn>(loadProc("glCheckFramebufferStatus"));
    glDeleteFramebuffers_ = reinterpret_cast<GLDeleteFramebuffersFn>(loadProc("glDeleteFramebuffers"));
    glDeleteTextures_ = reinterpret_cast<GLDeleteTexturesFn>(loadProc("glDeleteTextures"));
    glViewport_ = reinterpret_cast<GLViewportFn>(loadProc("glViewport"));
    return glCreateShader_ && glCreateProgram_ && glGenFramebuffers_ && glGenTextures_;
}

GLuint compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader_(type);
    glShaderSource_(shader, 1, &source, nullptr);
    glCompileShader_(shader);
    GLint ok = 0;
    glGetShaderiv_(shader, kGL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv_(shader, kGL_INFO_LOG_LENGTH, &len);
        std::string log(len > 1 ? len : 1, '\0');
        glGetShaderInfoLog_(shader, len, nullptr, log.data());
        MessageBoxA(nullptr, log.c_str(), "Shader compile error", MB_ICONERROR | MB_OK);
    }
    return shader;
}

GLuint createProgram(const char *vs, const char *fs) {
    GLuint program = glCreateProgram_();
    GLuint v = compileShader(kGL_VERTEX_SHADER, vs);
    GLuint f = compileShader(kGL_FRAGMENT_SHADER, fs);
    glAttachShader_(program, v);
    glAttachShader_(program, f);
    glLinkProgram_(program);
    GLint ok = 0;
    glGetProgramiv_(program, kGL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv_(program, kGL_INFO_LOG_LENGTH, &len);
        std::string log(len > 1 ? len : 1, '\0');
        glGetProgramInfoLog_(program, len, nullptr, log.data());
        MessageBoxA(nullptr, log.c_str(), "Program link error", MB_ICONERROR | MB_OK);
    }
    glDeleteShader_(v);
    glDeleteShader_(f);
    return program;
}
