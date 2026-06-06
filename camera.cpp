#include "camera.h"

#include <cmath>

static float clampf(float v, float a, float b) {
    return v < a ? a : (v > b ? b : v);
}

Vec3 cameraForward(float yaw, float pitch) {
    float cp = std::cos(pitch);
    return {cp * std::sin(yaw), std::sin(pitch), cp * std::cos(yaw)};
}

Vec3 cameraRight(float yaw) {
    return {std::cos(yaw), 0.0f, -std::sin(yaw)};
}

void updateCamera(Camera &camera, const CameraInput &input, float dt, TerrainHeightFn terrainHeight, float terrainTime, float terrainWidth, float terrainDepth, float clearance, float moveSpeed, float verticalSpeed, float turnSpeed) {
    camera.yaw += (input.turnRight ? 1.0f : 0.0f) * turnSpeed * dt;
    camera.yaw -= (input.turnLeft ? 1.0f : 0.0f) * turnSpeed * dt;
    camera.pitch += (input.turnUp ? 1.0f : 0.0f) * turnSpeed * dt;
    camera.pitch -= (input.turnDown ? 1.0f : 0.0f) * turnSpeed * dt;
    camera.pitch = clampf(camera.pitch, -1.35f, 1.0f);

    Vec3 forward = cameraForward(camera.yaw, camera.pitch);
    Vec3 right = cameraRight(camera.yaw);
    Vec3 velocity{0.0f, 0.0f, 0.0f};

    if (input.forward) { velocity.x += forward.x; velocity.y += forward.y; velocity.z += forward.z; }
    if (input.backward) { velocity.x -= forward.x; velocity.y -= forward.y; velocity.z -= forward.z; }
    if (input.right) { velocity.x += right.x; velocity.z += right.z; }
    if (input.left) { velocity.x -= right.x; velocity.z -= right.z; }
    if (input.up) velocity.y += 1.0f;
    if (input.down) velocity.y -= 1.0f;

    float len = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
    if (len > 0.0001f) {
        velocity.x /= len;
        velocity.y /= len;
        velocity.z /= len;
    }

    camera.x += velocity.x * moveSpeed * dt;
    camera.y += velocity.y * verticalSpeed * dt;
    camera.z += velocity.z * moveSpeed * dt;

    float halfW = terrainWidth * 0.5f;
    float halfD = terrainDepth * 0.5f;
    camera.x = clampf(camera.x, -halfW + 0.5f, halfW - 0.5f);
    camera.z = clampf(camera.z, -halfD + 0.5f, halfD - 0.5f);

    float ground = terrainHeight(camera.x, camera.z, terrainTime);
    float minY = ground + clearance;
    if (camera.y < minY) camera.y = minY;
}
