#include "math.h"

#include <cmath>

float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
float lerpf(float a, float b, float t) { return a + (b - a) * t; }

float smoothstep(float a, float b, float x) {
    float t = clampf((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

Vec3 vec3Add(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 vec3Subtract(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 vec3Scale(Vec3 v, float s) { return {v.x * s, v.y * s, v.z * s}; }
float vec3Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 vec3Cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec3 vec3Normalize(Vec3 v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len <= 0.00001f) return {0.0f, 1.0f, 0.0f};
    return {v.x / len, v.y / len, v.z / len};
}

Mat4 mat4Identity() {
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

Mat4 mat4Multiply(const Mat4 &a, const Mat4 &b) {
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

Mat4 mat4Inverse(const Mat4 &m) {
    Mat4 inv{};
    const float *a = m.m;
    float *o = inv.m;

    o[0] = a[5] * a[10] * a[15] - a[5] * a[11] * a[14] - a[9] * a[6] * a[15] + a[9] * a[7] * a[14] + a[13] * a[6] * a[11] - a[13] * a[7] * a[10];
    o[4] = -a[4] * a[10] * a[15] + a[4] * a[11] * a[14] + a[8] * a[6] * a[15] - a[8] * a[7] * a[14] - a[12] * a[6] * a[11] + a[12] * a[7] * a[10];
    o[8] = a[4] * a[9] * a[15] - a[4] * a[11] * a[13] - a[8] * a[5] * a[15] + a[8] * a[7] * a[13] + a[12] * a[5] * a[11] - a[12] * a[7] * a[9];
    o[12] = -a[4] * a[9] * a[14] + a[4] * a[10] * a[13] + a[8] * a[5] * a[14] - a[8] * a[6] * a[13] - a[12] * a[5] * a[10] + a[12] * a[6] * a[9];
    o[1] = -a[1] * a[10] * a[15] + a[1] * a[11] * a[14] + a[9] * a[2] * a[15] - a[9] * a[3] * a[14] - a[13] * a[2] * a[11] + a[13] * a[3] * a[10];
    o[5] = a[0] * a[10] * a[15] - a[0] * a[11] * a[14] - a[8] * a[2] * a[15] + a[8] * a[3] * a[14] + a[12] * a[2] * a[11] - a[12] * a[3] * a[10];
    o[9] = -a[0] * a[9] * a[15] + a[0] * a[11] * a[13] + a[8] * a[1] * a[15] - a[8] * a[3] * a[13] - a[12] * a[1] * a[11] + a[12] * a[3] * a[9];
    o[13] = a[0] * a[9] * a[14] - a[0] * a[10] * a[13] - a[8] * a[1] * a[14] + a[8] * a[2] * a[13] + a[12] * a[1] * a[10] - a[12] * a[2] * a[9];
    o[2] = a[1] * a[6] * a[15] - a[1] * a[7] * a[14] - a[5] * a[2] * a[15] + a[5] * a[3] * a[14] + a[13] * a[2] * a[7] - a[13] * a[3] * a[6];
    o[6] = -a[0] * a[6] * a[15] + a[0] * a[7] * a[14] + a[4] * a[2] * a[15] - a[4] * a[3] * a[14] - a[12] * a[2] * a[7] + a[12] * a[3] * a[6];
    o[10] = a[0] * a[5] * a[15] - a[0] * a[7] * a[13] - a[4] * a[1] * a[15] + a[4] * a[3] * a[13] + a[12] * a[1] * a[7] - a[12] * a[3] * a[5];
    o[14] = -a[0] * a[5] * a[14] + a[0] * a[6] * a[13] + a[4] * a[1] * a[14] - a[4] * a[2] * a[13] - a[12] * a[1] * a[6] + a[12] * a[2] * a[5];
    o[3] = -a[1] * a[6] * a[11] + a[1] * a[7] * a[10] + a[5] * a[2] * a[11] - a[5] * a[3] * a[10] - a[9] * a[2] * a[7] + a[9] * a[3] * a[6];
    o[7] = a[0] * a[6] * a[11] - a[0] * a[7] * a[10] - a[4] * a[2] * a[11] + a[4] * a[3] * a[10] + a[8] * a[2] * a[7] - a[8] * a[3] * a[6];
    o[11] = -a[0] * a[5] * a[11] + a[0] * a[7] * a[9] + a[4] * a[1] * a[11] - a[4] * a[3] * a[9] - a[8] * a[1] * a[7] + a[8] * a[3] * a[5];
    o[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10] + a[4] * a[2] * a[9] + a[8] * a[1] * a[6] - a[8] * a[2] * a[5];

    float det = a[0] * o[0] + a[1] * o[4] + a[2] * o[8] + a[3] * o[12];
    if (std::fabs(det) < 1e-8f) return mat4Identity();
    det = 1.0f / det;
    for (int i = 0; i < 16; ++i) o[i] *= det;
    return inv;
}

Mat4 mat4Perspective(float fovY, float aspect, float zNear, float zFar) {
    Mat4 r{};
    float f = 1.0f / std::tan(fovY * 0.5f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zFar + zNear) / (zNear - zFar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    return r;
}

Mat4 mat4Ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
    Mat4 r = mat4Identity();
    r.m[0] = 2.0f / (right - left);
    r.m[5] = 2.0f / (top - bottom);
    r.m[10] = -2.0f / (zFar - zNear);
    r.m[12] = -(right + left) / (right - left);
    r.m[13] = -(top + bottom) / (top - bottom);
    r.m[14] = -(zFar + zNear) / (zFar - zNear);
    return r;
}

Mat4 mat4LookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3Normalize(vec3Subtract(center, eye));
    Vec3 s = vec3Normalize(vec3Cross(f, up));
    Vec3 u = vec3Cross(s, f);
    Mat4 r = mat4Identity();
    r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
    r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
    r.m[12] = -vec3Dot(s, eye);
    r.m[13] = -vec3Dot(u, eye);
    r.m[14] = vec3Dot(f, eye);
    return r;
}

Mat4 mat4WithoutTranslation(const Mat4 &view) {
    Mat4 r = view;
    r.m[12] = r.m[13] = r.m[14] = 0.0f;
    return r;
}

Mat4 mat4Translation(Vec3 t) {
    Mat4 r = mat4Identity();
    r.m[12] = t.x;
    r.m[13] = t.y;
    r.m[14] = t.z;
    return r;
}

Mat4 mat4Scale(float s) {
    Mat4 r = mat4Identity();
    r.m[0] = r.m[5] = r.m[10] = s;
    return r;
}

Mat4 mat4RotationY(float angle) {
    Mat4 r = mat4Identity();
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    r.m[0] = c;
    r.m[8] = s;
    r.m[2] = -s;
    r.m[10] = c;
    return r;
}

Mat4 mat4Model(Vec3 pos, float rotY, float scale) {
    return mat4Multiply(mat4Translation(pos), mat4Multiply(mat4RotationY(rotY), mat4Scale(scale)));
}

Mat4 mat4FromQuat(Quat q) {
    q = quatNormalize(q);
    const float xx = q.x * q.x;
    const float yy = q.y * q.y;
    const float zz = q.z * q.z;
    const float xy = q.x * q.y;
    const float xz = q.x * q.z;
    const float yz = q.y * q.z;
    const float wx = q.w * q.x;
    const float wy = q.w * q.y;
    const float wz = q.w * q.z;

    Mat4 r = mat4Identity();
    r.m[0] = 1.0f - 2.0f * (yy + zz);
    r.m[1] = 2.0f * (xy + wz);
    r.m[2] = 2.0f * (xz - wy);
    r.m[4] = 2.0f * (xy - wz);
    r.m[5] = 1.0f - 2.0f * (xx + zz);
    r.m[6] = 2.0f * (yz + wx);
    r.m[8] = 2.0f * (xz + wy);
    r.m[9] = 2.0f * (yz - wx);
    r.m[10] = 1.0f - 2.0f * (xx + yy);
    return r;
}

Mat4 mat4ModelQuat(Vec3 pos, Quat rot, float scale) {
    return mat4Multiply(mat4Translation(pos), mat4Multiply(mat4FromQuat(rot), mat4Scale(scale)));
}

Quat quatNormalize(Quat q) {
    float len = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (len <= 0.00001f) return {0.0f, 0.0f, 0.0f, 1.0f};
    return {q.x / len, q.y / len, q.z / len, q.w / len};
}

Quat quatMultiply(Quat a, Quat b) {
    return {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    };
}

Quat quatFromAxisAngle(Vec3 axis, float angle) {
    axis = vec3Normalize(axis);
    float half = angle * 0.5f;
    float s = std::sin(half);
    return {axis.x * s, axis.y * s, axis.z * s, std::cos(half)};
}

Quat quatFromYawPitch(float yaw, float pitch) {
    Quat qYaw = quatFromAxisAngle({0.0f, 1.0f, 0.0f}, yaw);
    Quat qPitch = quatFromAxisAngle({1.0f, 0.0f, 0.0f}, pitch);
    return quatNormalize(quatMultiply(qYaw, qPitch));
}

Vec3 quatRotate(Quat q, Vec3 v) {
  q = quatNormalize(q);
    Quat p{v.x, v.y, v.z, 0.0f};
    Quat qi{-q.x, -q.y, -q.z, q.w};
    Quat r = quatMultiply(quatMultiply(q, p), qi);
    return {r.x, r.y, r.z};
}
