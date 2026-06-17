#pragma once

#include "gl_loader.h"

#include <string>

std::string loadShaderFile(const char *relativePath);
GLuint createProgramFromFiles(const char *vertPath, const char *fragPath);
