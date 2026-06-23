#pragma once

struct Vec3 {
    float x, y, z;
};

struct Mat4 {
    float m[16]{};
};

struct Quat {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

float clampf(float v, float a, float b);
float lerpf(float a, float b, float t);
float smoothstep(float a, float b, float x);

Vec3 vec3Add(Vec3 a, Vec3 b);
Vec3 vec3Subtract(Vec3 a, Vec3 b);
Vec3 vec3Scale(Vec3 v, float s);
float vec3Dot(Vec3 a, Vec3 b);
Vec3 vec3Cross(Vec3 a, Vec3 b);
Vec3 vec3Normalize(Vec3 v);

Mat4 mat4Identity();
Mat4 mat4Multiply(const Mat4 &a, const Mat4 &b);
Mat4 mat4Perspective(float fovY, float aspect, float zNear, float zFar);
Mat4 mat4Ortho(float left, float right, float bottom, float top, float zNear, float zFar);
Mat4 mat4LookAt(Vec3 eye, Vec3 center, Vec3 up);
Mat4 mat4WithoutTranslation(const Mat4 &view);

Mat4 mat4Translation(Vec3 t);
Mat4 mat4Scale(float s);
Mat4 mat4RotationY(float angle);
Mat4 mat4Model(Vec3 pos, float rotY, float scale);
Mat4 mat4FromQuat(Quat q);
Mat4 mat4ModelQuat(Vec3 pos, Quat rot, float scale);

Quat quatNormalize(Quat q);
Quat quatMultiply(Quat a, Quat b);
Quat quatFromAxisAngle(Vec3 axis, float angle);
Quat quatFromYawPitch(float yaw, float pitch);
Vec3 quatRotate(Quat q, Vec3 v);
