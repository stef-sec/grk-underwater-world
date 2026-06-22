#pragma once

#include "math.h"

struct Camera {
    float x = 0.0f;
    float y = -1.0f;
    float z = 16.0f;
    Quat orientation = quatFromYawPitch(3.14159265f, -0.15f);
};

struct CameraInput {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool turnLeft = false;
    bool turnRight = false;
    bool turnUp = false;
    bool turnDown = false;
};

using TerrainHeightFn = float (*)(float x, float z, float time);

Vec3 cameraForward(const Camera &camera);
Vec3 cameraRight(const Camera &camera);
void updateCamera(Camera &camera, const CameraInput &input, float dt, TerrainHeightFn terrainHeight, float terrainTime, float terrainWidth, float terrainDepth, float groundClearance, float waterSurface, float surfaceClearance, float moveSpeed, float verticalSpeed, float turnSpeed);
