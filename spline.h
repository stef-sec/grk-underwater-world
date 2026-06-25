#pragma once

#include "math.h"

#include <vector>

// Catmull-Rom spline closed loop through control points.
struct SplinePath {
    std::vector<Vec3> points;
    std::vector<float> cumulativeLength;
    float totalLength = 0.0f;
};

// Parallel Transport Frame (Bishop frame) — stable orientation on curved paths.
struct ParallelTransportFrame {
    Vec3 tangent{0.0f, 0.0f, 1.0f};
    Vec3 normal{0.0f, 1.0f, 0.0f};
    Vec3 binormal{1.0f, 0.0f, 0.0f};
};

void buildSplinePath(SplinePath &path, const Vec3 *controlPoints, int count);
Vec3 splinePosition(const SplinePath &path, float t);
Vec3 splineTangent(const SplinePath &path, float t);

void initParallelTransportFrame(ParallelTransportFrame &frame, Vec3 tangent, Vec3 referenceUp);
void transportParallelFrame(ParallelTransportFrame &frame, Vec3 newTangent);
Mat4 mat4ModelPTF(Vec3 position, const ParallelTransportFrame &frame, float scale);
