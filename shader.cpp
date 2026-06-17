#include "shader.h"

#include "gl_loader.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static std::string readTextFile(const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::string exeDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH]{};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::string path(buffer);
    const size_t slash = path.find_last_of("\\/");
    if (slash != std::string::npos) path.resize(slash + 1);
    return path;
#else
    return "";
#endif
}

std::string loadShaderFile(const char *relativePath) {
    const std::string base = exeDirectory();
    const std::vector<std::string> candidates = {
        base + relativePath,
        std::string(relativePath),
        std::string("../") + relativePath,
        std::string("../../") + relativePath,
    };
    for (const std::string &path : candidates) {
        std::string text = readTextFile(path);
        if (!text.empty()) return text;
    }
    MessageBoxA(nullptr, relativePath, "Shader file not found", MB_ICONERROR | MB_OK);
    return "void main(){}\n";
}

GLuint createProgramFromFiles(const char *vertPath, const char *fragPath) {
    const std::string vs = loadShaderFile(vertPath);
    const std::string fs = loadShaderFile(fragPath);
    return createProgram(vs.c_str(), fs.c_str());
}
