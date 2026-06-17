#pragma once

struct Vec3 {
    float x, y, z;
};

struct Camera {
    float x = 0.0f;
    float y = -1.0f;
    float z = 16.0f;
    float yaw = 3.14159265f;
    float pitch = -0.15f;
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

Vec3 cameraForward(float yaw, float pitch);
Vec3 cameraRight(float yaw);
void updateCamera(Camera &camera, const CameraInput &input, float dt, TerrainHeightFn terrainHeight, float terrainTime, float terrainWidth, float terrainDepth, float groundClearance, float waterSurface, float surfaceClearance, float moveSpeed, float verticalSpeed, float turnSpeed);
