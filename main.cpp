#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "terrain.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

static constexpr int kWidth = 1280;
static constexpr int kHeight = 720;
static constexpr float kCameraClearance = 0.6f;
static constexpr float kMoveSpeed = 9.0f;
static constexpr float kVerticalSpeed = 6.0f;
static constexpr float kTurnSpeed = 1.6f;

using GLuint = unsigned int;
using GLint = int;
using GLenum = unsigned int;
using GLsizei = int;
using GLchar = char;
using GLsizeiptr = ptrdiff_t;
using GLfloat = float;
using GLboolean = unsigned char;

static constexpr GLenum kGL_VERTEX_SHADER = 0x8B31;
static constexpr GLenum kGL_FRAGMENT_SHADER = 0x8B30;
static constexpr GLenum kGL_COMPILE_STATUS = 0x8B81;
static constexpr GLenum kGL_LINK_STATUS = 0x8B82;
static constexpr GLenum kGL_INFO_LOG_LENGTH = 0x8B84;
static constexpr GLenum kGL_ARRAY_BUFFER = 0x8892;
static constexpr GLenum kGL_STATIC_DRAW = 0x88E4;
static constexpr GLenum kGL_TRIANGLES = 0x0004;
static constexpr GLenum kGL_COLOR_BUFFER_BIT = 0x00004000;
static constexpr GLenum kGL_DEPTH_BUFFER_BIT = 0x00000100;
static constexpr GLenum kGL_DEPTH_TEST = 0x0B71;

using MyGLCreateShaderProc = GLuint(__stdcall *)(GLenum);
using MyGLShaderSourceProc = void(__stdcall *)(GLuint, GLsizei, const GLchar *const *, const GLint *);
using MyGLCompileShaderProc = void(__stdcall *)(GLuint);
using MyGLGetShaderivProc = void(__stdcall *)(GLuint, GLenum, GLint *);
using MyGLGetShaderInfoLogProc = void(__stdcall *)(GLuint, GLsizei, GLsizei *, GLchar *);
using MyGLCreateProgramProc = GLuint(__stdcall *)();
using MyGLAttachShaderProc = void(__stdcall *)(GLuint, GLuint);
using MyGLLinkProgramProc = void(__stdcall *)(GLuint);
using MyGLGetProgramivProc = void(__stdcall *)(GLuint, GLenum, GLint *);
using MyGLGetProgramInfoLogProc = void(__stdcall *)(GLuint, GLsizei, GLsizei *, GLchar *);
using MyGLUseProgramProc = void(__stdcall *)(GLuint);
using MyGLDeleteShaderProc = void(__stdcall *)(GLuint);
using MyGLDeleteProgramProc = void(__stdcall *)(GLuint);
using MyGLGenBuffersProc = void(__stdcall *)(GLsizei, GLuint *);
using MyGLBindBufferProc = void(__stdcall *)(GLenum, GLuint);
using MyGLBufferDataProc = void(__stdcall *)(GLenum, GLsizeiptr, const void *, GLenum);
using MyGLGenVertexArraysProc = void(__stdcall *)(GLsizei, GLuint *);
using MyGLBindVertexArrayProc = void(__stdcall *)(GLuint);
using MyGLEnableVertexAttribArrayProc = void(__stdcall *)(GLuint);
using MyGLVertexAttribPointerProc = void(__stdcall *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
using MyGLGetUniformLocationProc = GLint(__stdcall *)(GLuint, const GLchar *);
using MyGLUniform1fProc = void(__stdcall *)(GLint, GLfloat);
using MyGLUniform3fProc = void(__stdcall *)(GLint, GLfloat, GLfloat, GLfloat);
using MyGLUniformMatrix4fvProc = void(__stdcall *)(GLint, GLsizei, GLboolean, const GLfloat *);

MyGLCreateShaderProc glCreateShader_ = nullptr;
MyGLShaderSourceProc glShaderSource_ = nullptr;
MyGLCompileShaderProc glCompileShader_ = nullptr;
MyGLGetShaderivProc glGetShaderiv_ = nullptr;
MyGLGetShaderInfoLogProc glGetShaderInfoLog_ = nullptr;
MyGLCreateProgramProc glCreateProgram_ = nullptr;
MyGLAttachShaderProc glAttachShader_ = nullptr;
MyGLLinkProgramProc glLinkProgram_ = nullptr;
MyGLGetProgramivProc glGetProgramiv_ = nullptr;
MyGLGetProgramInfoLogProc glGetProgramInfoLog_ = nullptr;
MyGLUseProgramProc glUseProgram_ = nullptr;
MyGLDeleteShaderProc glDeleteShader_ = nullptr;
MyGLDeleteProgramProc glDeleteProgram_ = nullptr;
MyGLGenBuffersProc glGenBuffers_ = nullptr;
MyGLBindBufferProc glBindBuffer_ = nullptr;
MyGLBufferDataProc glBufferData_ = nullptr;
MyGLGenVertexArraysProc glGenVertexArrays_ = nullptr;
MyGLBindVertexArrayProc glBindVertexArray_ = nullptr;
MyGLEnableVertexAttribArrayProc glEnableVertexAttribArray_ = nullptr;
MyGLVertexAttribPointerProc glVertexAttribPointer_ = nullptr;
MyGLGetUniformLocationProc glGetUniformLocation_ = nullptr;
MyGLUniform1fProc glUniform1f_ = nullptr;
MyGLUniform3fProc glUniform3f_ = nullptr;
MyGLUniformMatrix4fvProc glUniformMatrix4fv_ = nullptr;

struct Vec3 {
	float x, y, z;
};

struct Mat4 {
	float m[16]{};
};

struct Camera {
	float x = 0.0f;
	float y = -1.0f;
	float z = 16.0f;
	float yaw = 3.14159265f;
	float pitch = -0.15f;
};

static HDC g_hdc = nullptr;
static HGLRC g_hrc = nullptr;
static HWND g_hwnd = nullptr;
static bool g_running = true;
static TerrainGPU g_terrain;
static GLuint g_program = 0;
static GLint g_uViewProj = -1;
static GLint g_uTime = -1;
static GLint g_uWaterLevel = -1;
static GLint g_uFogDensity = -1;
static GLint g_uBaseColor = -1;
static GLint g_uDeepColor = -1;
static Camera g_camera;
static bool g_keyW = false, g_keyA = false, g_keyS = false, g_keyD = false;
static bool g_keyQ = false, g_keyE = false, g_keyUp = false, g_keyDown = false, g_keyLeft = false, g_keyRight = false;
static float g_time = 0.0f;
static float g_waterLevel = 0.8f;
static float g_fogDensity = 0.07f;

static float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
static float lerpf(float a, float b, float t) { return a + (b - a) * t; }
static float smoothstep(float a, float b, float x) {
	float t = clampf((x - a) / (b - a), 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

static Vec3 normalize(Vec3 v) {
	float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (len <= 0.00001f) return {0.0f, 1.0f, 0.0f};
	return {v.x / len, v.y / len, v.z / len};
}

static Vec3 cross(Vec3 a, Vec3 b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static Vec3 subtract(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static Vec3 forwardFromAngles(float yaw, float pitch) {
	float cp = std::cos(pitch);
	return {cp * std::sin(yaw), std::sin(pitch), cp * std::cos(yaw)};
}

static Vec3 rightFromYaw(float yaw) { return {std::cos(yaw), 0.0f, -std::sin(yaw)}; }

static Mat4 identity() {
	Mat4 r{};
	r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
	return r;
}

static Mat4 multiply(const Mat4 &a, const Mat4 &b) {
	Mat4 r{};
	for (int c = 0; c < 4; ++c) {
		for (int row = 0; row < 4; ++row) {
			for (int k = 0; k < 4; ++k) {
				r.m[c * 4 + row] += a.m[k * 4 + row] * b.m[c * 4 + k];
			}
		}
	}
	return r;
}

static Mat4 perspective(float fovY, float aspect, float zNear, float zFar) {
	Mat4 r{};
	float f = 1.0f / std::tan(fovY * 0.5f);
	r.m[0] = f / aspect;
	r.m[5] = f;
	r.m[10] = (zFar + zNear) / (zNear - zFar);
	r.m[11] = -1.0f;
	r.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
	return r;
}

static Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
	Vec3 f = normalize(subtract(center, eye));
	Vec3 s = normalize(cross(f, up));
	Vec3 u = cross(s, f);
	Mat4 r = identity();
	r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
	r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
	r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
	r.m[12] = -dot(s, eye);
	r.m[13] = -dot(u, eye);
	r.m[14] = dot(f, eye);
	return r;
}

static void loadGLFunctions() {
	auto load = [](const char *name) -> void * {
		void *p = reinterpret_cast<void *>(wglGetProcAddress(name));
		if (!p) {
			HMODULE mod = GetModuleHandleA("opengl32.dll");
			p = reinterpret_cast<void *>(GetProcAddress(mod, name));
		}
		return p;
	};
  glCreateShader_ = reinterpret_cast<MyGLCreateShaderProc>(load("glCreateShader"));
	glShaderSource_ = reinterpret_cast<MyGLShaderSourceProc>(load("glShaderSource"));
	glCompileShader_ = reinterpret_cast<MyGLCompileShaderProc>(load("glCompileShader"));
	glGetShaderiv_ = reinterpret_cast<MyGLGetShaderivProc>(load("glGetShaderiv"));
	glGetShaderInfoLog_ = reinterpret_cast<MyGLGetShaderInfoLogProc>(load("glGetShaderInfoLog"));
	glCreateProgram_ = reinterpret_cast<MyGLCreateProgramProc>(load("glCreateProgram"));
	glAttachShader_ = reinterpret_cast<MyGLAttachShaderProc>(load("glAttachShader"));
	glLinkProgram_ = reinterpret_cast<MyGLLinkProgramProc>(load("glLinkProgram"));
	glGetProgramiv_ = reinterpret_cast<MyGLGetProgramivProc>(load("glGetProgramiv"));
	glGetProgramInfoLog_ = reinterpret_cast<MyGLGetProgramInfoLogProc>(load("glGetProgramInfoLog"));
	glUseProgram_ = reinterpret_cast<MyGLUseProgramProc>(load("glUseProgram"));
	glDeleteShader_ = reinterpret_cast<MyGLDeleteShaderProc>(load("glDeleteShader"));
	glDeleteProgram_ = reinterpret_cast<MyGLDeleteProgramProc>(load("glDeleteProgram"));
	glGenBuffers_ = reinterpret_cast<MyGLGenBuffersProc>(load("glGenBuffers"));
	glBindBuffer_ = reinterpret_cast<MyGLBindBufferProc>(load("glBindBuffer"));
	glBufferData_ = reinterpret_cast<MyGLBufferDataProc>(load("glBufferData"));
	glGenVertexArrays_ = reinterpret_cast<MyGLGenVertexArraysProc>(load("glGenVertexArrays"));
	glBindVertexArray_ = reinterpret_cast<MyGLBindVertexArrayProc>(load("glBindVertexArray"));
	glEnableVertexAttribArray_ = reinterpret_cast<MyGLEnableVertexAttribArrayProc>(load("glEnableVertexAttribArray"));
	glVertexAttribPointer_ = reinterpret_cast<MyGLVertexAttribPointerProc>(load("glVertexAttribPointer"));
	glGetUniformLocation_ = reinterpret_cast<MyGLGetUniformLocationProc>(load("glGetUniformLocation"));
	glUniform1f_ = reinterpret_cast<MyGLUniform1fProc>(load("glUniform1f"));
	glUniform3f_ = reinterpret_cast<MyGLUniform3fProc>(load("glUniform3f"));
	glUniformMatrix4fv_ = reinterpret_cast<MyGLUniformMatrix4fvProc>(load("glUniformMatrix4fv"));
}

static GLuint compileShader(GLenum type, const char *source) {
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

static GLuint createProgram(const char *vs, const char *fs) {
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

static void initScene() {
	const char *vs = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in float aFoam;

uniform mat4 uViewProj;
uniform float uTime;

out vec3 vWorldPos;
out vec3 vNormal;
out float vFoam;

void main() {
    vec3 pos = aPos;
    vec3 normal = normalize(aNormal);
    vWorldPos = pos;
    vNormal = normal;
    vFoam = aFoam;
    gl_Position = uViewProj * vec4(pos, 1.0);
}
)GLSL";

	const char *fs = R"GLSL(
#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in float vFoam;

uniform float uTime;
uniform float uWaterLevel;
uniform float uFogDensity;
uniform vec3 uBaseColor;
uniform vec3 uDeepColor;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(-0.2, 1.0, 0.15));
    float diffuse = max(dot(N, L), 0.0);
    float ambient = 0.16;
    float foam = smoothstep(0.45, 0.92, vFoam + max(vWorldPos.y - (-8.0), 0.0) * 0.02);
    float depthFactor = clamp((uWaterLevel - vWorldPos.y) / 22.0, 0.0, 1.0);
    float fog = 1.0 - exp(-uFogDensity * depthFactor * depthFactor * 18.0);
    vec3 waterScatter = mix(vec3(0.06, 0.22, 0.32), vec3(0.01, 0.08, 0.14), depthFactor);
    vec3 lit = uBaseColor * (ambient + diffuse * 0.95);
    lit = mix(lit, uDeepColor, fog);
    lit = mix(lit, waterScatter, clamp(depthFactor * 0.55, 0.0, 1.0));
    lit += foam * vec3(0.07, 0.12, 0.13);
    float caustic = 0.5 + 0.5 * sin(vWorldPos.x * 0.9 + uTime * 1.9) * sin(vWorldPos.z * 0.8 - uTime * 1.4);
    lit += caustic * (1.0 - depthFactor) * 0.04;
    FragColor = vec4(lit, 1.0);
}
)GLSL";

	g_program = createProgram(vs, fs);
	g_uViewProj = glGetUniformLocation_(g_program, "uViewProj");
	g_uTime = glGetUniformLocation_(g_program, "uTime");
	g_uWaterLevel = glGetUniformLocation_(g_program, "uWaterLevel");
	g_uFogDensity = glGetUniformLocation_(g_program, "uFogDensity");
	g_uBaseColor = glGetUniformLocation_(g_program, "uBaseColor");
	g_uDeepColor = glGetUniformLocation_(g_program, "uDeepColor");
   buildTerrain(g_terrain);
	glEnable(kGL_DEPTH_TEST);
}

static void resizeViewport(int width, int height) {
	if (height <= 0) height = 1;
	glViewport(0, 0, width, height);
}

static void updateInput(float dt) {
	g_camera.yaw += (g_keyRight ? 1.0f : 0.0f) * kTurnSpeed * dt;
	g_camera.yaw -= (g_keyLeft ? 1.0f : 0.0f) * kTurnSpeed * dt;
	g_camera.pitch += (g_keyUp ? 1.0f : 0.0f) * kTurnSpeed * dt;
	g_camera.pitch -= (g_keyDown ? 1.0f : 0.0f) * kTurnSpeed * dt;
	g_camera.pitch = clampf(g_camera.pitch, -1.35f, 1.0f);

	Vec3 forward = forwardFromAngles(g_camera.yaw, g_camera.pitch);
	Vec3 right = rightFromYaw(g_camera.yaw);
	Vec3 velocity{0.0f, 0.0f, 0.0f};

	if (g_keyW) { velocity.x += forward.x; velocity.y += forward.y; velocity.z += forward.z; }
	if (g_keyS) { velocity.x -= forward.x; velocity.y -= forward.y; velocity.z -= forward.z; }
	if (g_keyD) { velocity.x += right.x; velocity.z += right.z; }
	if (g_keyA) { velocity.x -= right.x; velocity.z -= right.z; }
	if (g_keyE) velocity.y += 1.0f;
	if (g_keyQ) velocity.y -= 1.0f;

	float len = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
	if (len > 0.0001f) {
		velocity.x /= len;
		velocity.y /= len;
		velocity.z /= len;
	}

	g_camera.x += velocity.x * kMoveSpeed * dt;
	g_camera.y += velocity.y * kVerticalSpeed * dt;
	g_camera.z += velocity.z * kMoveSpeed * dt;

	float halfW = kTerrainWidth * 0.5f;
	float halfD = kTerrainDepth * 0.5f;
	g_camera.x = clampf(g_camera.x, -halfW + 0.5f, halfW - 0.5f);
	g_camera.z = clampf(g_camera.z, -halfD + 0.5f, halfD - 0.5f);

   float ground = terrainHeight(g_camera.x, g_camera.z, 0.0f);
	float minY = ground + kCameraClearance;
	if (g_camera.y < minY) g_camera.y = minY;

	if (GetAsyncKeyState(VK_PRIOR) & 0x8000) g_waterLevel += 1.2f * dt;
	if (GetAsyncKeyState(VK_NEXT) & 0x8000) g_waterLevel -= 1.2f * dt;
	if (GetAsyncKeyState('Z') & 0x8000) g_fogDensity = clampf(g_fogDensity - 0.3f * dt, 0.01f, 0.4f);
	if (GetAsyncKeyState('X') & 0x8000) g_fogDensity = clampf(g_fogDensity + 0.3f * dt, 0.01f, 0.4f);
}

static void renderFrame() {
	RECT rc{};
	GetClientRect(g_hwnd, &rc);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	resizeViewport(width, height);

	glClearColor(0.01f, 0.07f, 0.12f, 1.0f);
	glClear(kGL_COLOR_BUFFER_BIT | kGL_DEPTH_BUFFER_BIT);

	Vec3 eye{g_camera.x, g_camera.y, g_camera.z};
	Vec3 forward = forwardFromAngles(g_camera.yaw, g_camera.pitch);
	Mat4 proj = perspective(60.0f * 3.14159265f / 180.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 200.0f);
	Mat4 view = lookAt(eye, {eye.x + forward.x, eye.y + forward.y, eye.z + forward.z}, {0.0f, 1.0f, 0.0f});
	Mat4 vp = multiply(proj, view);

	glUseProgram_(g_program);
	glUniformMatrix4fv_(g_uViewProj, 1, 0, vp.m);
	glUniform1f_(g_uTime, g_time);
	glUniform1f_(g_uWaterLevel, g_waterLevel);
	glUniform1f_(g_uFogDensity, g_fogDensity);
	glUniform3f_(g_uBaseColor, 0.55f, 0.46f, 0.26f);
	glUniform3f_(g_uDeepColor, 0.02f, 0.11f, 0.16f);

	glBindVertexArray_(g_terrain.vao);
	glDrawArrays(kGL_TRIANGLES, 0, static_cast<GLsizei>(g_terrain.count));
	glBindVertexArray_(0);
	glUseProgram_(0);
	SwapBuffers(g_hdc);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE:
	case WM_DESTROY:
		g_running = false;
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		resizeViewport(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
		case 'W': g_keyW = true; break;
		case 'A': g_keyA = true; break;
		case 'S': g_keyS = true; break;
		case 'D': g_keyD = true; break;
		case 'Q': g_keyQ = true; break;
		case 'E': g_keyE = true; break;
		case VK_LEFT: g_keyLeft = true; break;
		case VK_RIGHT: g_keyRight = true; break;
		case VK_UP: g_keyUp = true; break;
		case VK_DOWN: g_keyDown = true; break;
		case VK_ESCAPE: g_running = false; PostQuitMessage(0); break;
		default: break;
		}
		return 0;
	case WM_KEYUP:
		switch (wParam) {
		case 'W': g_keyW = false; break;
		case 'A': g_keyA = false; break;
		case 'S': g_keyS = false; break;
		case 'D': g_keyD = false; break;
		case 'Q': g_keyQ = false; break;
		case 'E': g_keyE = false; break;
		case VK_LEFT: g_keyLeft = false; break;
		case VK_RIGHT: g_keyRight = false; break;
		case VK_UP: g_keyUp = false; break;
		case VK_DOWN: g_keyDown = false; break;
		default: break;
		}
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

static bool initOpenGL(HWND hwnd) {
	PIXELFORMATDESCRIPTOR pfd{};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	g_hdc = GetDC(hwnd);
	int pf = ChoosePixelFormat(g_hdc, &pfd);
	if (!pf || !SetPixelFormat(g_hdc, pf, &pfd)) return false;

	HGLRC temp = wglCreateContext(g_hdc);
	if (!temp || !wglMakeCurrent(g_hdc, temp)) return false;
	loadGLFunctions();
	initScene();
	g_hrc = temp;
	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	WNDCLASSA wc{};
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "UnderwaterWorldWindow";
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClassA(&wc);

	RECT rect{0, 0, kWidth, kHeight};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	g_hwnd = CreateWindowA(wc.lpszClassName, "GRK Underwater World", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, hInstance, nullptr);
	if (!g_hwnd) return 1;
	if (!initOpenGL(g_hwnd)) return 2;
	ShowWindow(g_hwnd, nCmdShow);
	UpdateWindow(g_hwnd);

	MSG msg{};
	DWORD last = GetTickCount();
	while (g_running) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!g_running) break;
		DWORD now = GetTickCount();
		float dt = static_cast<float>(now - last) / 1000.0f;
		last = now;
		dt = clampf(dt, 0.0f, 0.033f);
		g_time += dt;
		updateInput(dt);
		renderFrame();
		Sleep(1);
	}

	if (g_program) glDeleteProgram_(g_program);
	if (g_hrc) {
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(g_hrc);
	}
  destroyTerrain(g_terrain);
	if (g_hwnd && g_hdc) ReleaseDC(g_hwnd, g_hdc);
	return 0;
}

int main() {
	return WinMain(GetModuleHandleA(nullptr), nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
}