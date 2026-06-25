#include "spline.h"

#include <cmath>

static int wrapIndex(int i, int n) {
    if (n <= 0) return 0;
    i %= n;
    if (i < 0) i += n;
    return i;
}

static Vec3 catmullRomPoint(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, float u) {
    const float u2 = u * u;
    const float u3 = u2 * u;
    return {
        0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * u + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * u2 + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * u3),
        0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * u + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * u2 + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * u3),
        0.5f * ((2.0f * p1.z) + (-p0.z + p2.z) * u + (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * u2 + (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * u3),
    };
}

static Vec3 catmullRomTangent(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, float u) {
    const float u2 = u * u;
    return {
        0.5f * ((-p0.x + p2.x) + 2.0f * (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * u + 3.0f * (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * u2),
        0.5f * ((-p0.y + p2.y) + 2.0f * (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * u + 3.0f * (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * u2),
        0.5f * ((-p0.z + p2.z) + 2.0f * (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * u + 3.0f * (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * u2),
    };
}

static void splineSegment(const SplinePath &path, float t, int &seg, float &u) {
    const int n = static_cast<int>(path.points.size());
    if (n < 2) {
        seg = 0;
        u = 0.0f;
        return;
    }
    t = t - std::floor(t);
    const float scaled = t * static_cast<float>(n);
    seg = static_cast<int>(scaled) % n;
    u = scaled - static_cast<float>(seg);
}

void buildSplinePath(SplinePath &path, const Vec3 *controlPoints, int count) {
    path.points.assign(controlPoints, controlPoints + count);
    path.cumulativeLength.clear();
    path.totalLength = 0.0f;
    if (count < 2) return;

    constexpr int kSamples = 32;
    path.cumulativeLength.push_back(0.0f);
    Vec3 prev = splinePosition(path, 0.0f);
    for (int i = 1; i <= count * kSamples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(count * kSamples);
        const Vec3 cur = splinePosition(path, t);
        path.totalLength += std::sqrt(vec3Dot(vec3Subtract(cur, prev), vec3Subtract(cur, prev)));
        path.cumulativeLength.push_back(path.totalLength);
        prev = cur;
    }
}

Vec3 splinePosition(const SplinePath &path, float t) {
    const int n = static_cast<int>(path.points.size());
    if (n == 0) return {};
    if (n == 1) return path.points[0];

    int seg = 0;
    float u = 0.0f;
    splineSegment(path, t, seg, u);

    const Vec3 p0 = path.points[wrapIndex(seg - 1, n)];
    const Vec3 p1 = path.points[wrapIndex(seg, n)];
    const Vec3 p2 = path.points[wrapIndex(seg + 1, n)];
    const Vec3 p3 = path.points[wrapIndex(seg + 2, n)];
    return catmullRomPoint(p0, p1, p2, p3, u);
}

Vec3 splineTangent(const SplinePath &path, float t) {
    const int n = static_cast<int>(path.points.size());
    if (n < 2) return {0.0f, 0.0f, 1.0f};

    int seg = 0;
    float u = 0.0f;
    splineSegment(path, t, seg, u);

    const Vec3 p0 = path.points[wrapIndex(seg - 1, n)];
    const Vec3 p1 = path.points[wrapIndex(seg, n)];
    const Vec3 p2 = path.points[wrapIndex(seg + 1, n)];
    const Vec3 p3 = path.points[wrapIndex(seg + 2, n)];
    return vec3Normalize(catmullRomTangent(p0, p1, p2, p3, u));
}

void initParallelTransportFrame(ParallelTransportFrame &frame, Vec3 tangent, Vec3 referenceUp) {
    frame.tangent = vec3Normalize(tangent);
    Vec3 up = vec3Normalize(referenceUp);
    if (std::fabs(vec3Dot(frame.tangent, up)) > 0.95f) {
        up = {0.0f, 0.0f, 1.0f};
    }
    frame.binormal = vec3Normalize(vec3Cross(up, frame.tangent));
    frame.normal = vec3Normalize(vec3Cross(frame.tangent, frame.binormal));
}

void transportParallelFrame(ParallelTransportFrame &frame, Vec3 newTangent) {
    newTangent = vec3Normalize(newTangent);
    const Vec3 axis = vec3Cross(frame.tangent, newTangent);
    const float axisLen = std::sqrt(vec3Dot(axis, axis));
    if (axisLen > 1e-5f) {
        const Vec3 rotAxis = vec3Scale(axis, 1.0f / axisLen);
        const float dot = clampf(vec3Dot(frame.tangent, newTangent), -1.0f, 1.0f);
        const float angle = std::acos(dot);
        const Quat q = quatFromAxisAngle(rotAxis, angle);
        frame.normal = quatRotate(q, frame.normal);
        frame.binormal = quatRotate(q, frame.binormal);
    }
    frame.tangent = newTangent;
    frame.normal = vec3Normalize(frame.normal);
    frame.binormal = vec3Normalize(vec3Cross(frame.tangent, frame.normal));
    frame.normal = vec3Normalize(vec3Cross(frame.binormal, frame.tangent));
}

Mat4 mat4ModelPTF(Vec3 position, const ParallelTransportFrame &frame, float scale) {
    Mat4 r = mat4Identity();
    r.m[0] = frame.tangent.x * scale;
    r.m[1] = frame.tangent.y * scale;
    r.m[2] = frame.tangent.z * scale;
    r.m[4] = frame.normal.x * scale;
    r.m[5] = frame.normal.y * scale;
    r.m[6] = frame.normal.z * scale;
    r.m[8] = frame.binormal.x * scale;
    r.m[9] = frame.binormal.y * scale;
    r.m[10] = frame.binormal.z * scale;
    r.m[12] = position.x;
    r.m[13] = position.y;
    r.m[14] = position.z;
    return r;
}
