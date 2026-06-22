#include "camera.h"

#include <cmath>

Vec3 cameraForward(const Camera &camera) {
    return quatRotate(camera.orientation, {0.0f, 0.0f, -1.0f});
}

Vec3 cameraRight(const Camera &camera) {
    return quatRotate(camera.orientation, {1.0f, 0.0f, 0.0f});
}

void updateCamera(Camera &camera, const CameraInput &input, float dt, TerrainHeightFn terrainHeight, float terrainTime, float terrainWidth, float terrainDepth, float groundClearance, float waterSurface, float surfaceClearance, float moveSpeed, float verticalSpeed, float turnSpeed) {
    const float yawDelta = ((input.turnRight ? 1.0f : 0.0f) - (input.turnLeft ? 1.0f : 0.0f)) * turnSpeed * dt;
    const float pitchDelta = ((input.turnUp ? 1.0f : 0.0f) - (input.turnDown ? 1.0f : 0.0f)) * turnSpeed * dt;

    if (std::fabs(yawDelta) > 0.00001f) {
        Quat qYaw = quatFromAxisAngle({0.0f, 1.0f, 0.0f}, yawDelta);
        camera.orientation = quatNormalize(quatMultiply(qYaw, camera.orientation));
    }
    if (std::fabs(pitchDelta) > 0.00001f) {
        Vec3 right = cameraRight(camera);
        Quat qPitch = quatFromAxisAngle(right, pitchDelta);
        Quat trial = quatNormalize(quatMultiply(qPitch, camera.orientation));
        Vec3 trialFwd = quatRotate(trial, {0.0f, 0.0f, -1.0f});
        if (trialFwd.y <= 0.75f && trialFwd.y >= -0.92f) {
            camera.orientation = trial;
        }
    }

    Vec3 forward = cameraForward(camera);
    Vec3 right = cameraRight(camera);
    Vec3 velocity{0.0f, 0.0f, 0.0f};

    if (input.forward) velocity = vec3Add(velocity, forward);
    if (input.backward) velocity = vec3Subtract(velocity, forward);
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

    const float halfW = terrainWidth * 0.5f;
    const float halfD = terrainDepth * 0.5f;
    camera.x = clampf(camera.x, -halfW + 0.5f, halfW - 0.5f);
    camera.z = clampf(camera.z, -halfD + 0.5f, halfD - 0.5f);

    const float ground = terrainHeight(camera.x, camera.z, terrainTime);
    const float minY = ground + groundClearance;
    const float maxY = waterSurface - surfaceClearance;
    camera.y = clampf(camera.y, minY, maxY);
}
