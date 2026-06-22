#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uViewProj;
uniform float uTime;
uniform float uWaterSurfaceLevel;

out float vWave;
out vec3 vNormal;
out vec3 vWorldPos;
out float vSteepness;

const float PI = 3.14159265;

void gerstner(vec2 xz, vec2 dir, float amp, float wavelength, float speed, float steep,
              inout vec3 pos, inout vec3 tangentX, inout vec3 tangentZ) {
    dir = normalize(dir);
    float k = 2.0 * PI / wavelength;
    float omega = speed * sqrt(9.8 * k) * 0.38;
    float phase = k * dot(dir, xz) - omega * uTime;
    float c = cos(phase);
    float s = sin(phase);

    pos.x += steep * amp * dir.x * c;
    pos.y += amp * s;
    pos.z += steep * amp * dir.y * c;

    tangentX.x += -steep * amp * dir.x * dir.x * k * s;
    tangentX.y += amp * dir.x * k * c;
    tangentX.z += -steep * amp * dir.y * dir.x * k * s;

    tangentZ.x += -steep * amp * dir.x * dir.y * k * s;
    tangentZ.y += amp * dir.y * k * c;
    tangentZ.z += -steep * amp * dir.y * dir.y * k * s;
}

void main() {
    vec3 pos = aPos;
    vec3 tangentX = vec3(1.0, 0.0, 0.0);
    vec3 tangentZ = vec3(0.0, 0.0, 1.0);

    gerstner(pos.xz, vec2(1.0, 0.35),  0.28, 16.0, 0.75, 0.55, pos, tangentX, tangentZ);
    gerstner(pos.xz, vec2(-0.55, 0.85), 0.20, 10.5, 0.95, 0.50, pos, tangentX, tangentZ);
    gerstner(pos.xz, vec2(0.40, -0.92), 0.14,  7.0, 1.10, 0.42, pos, tangentX, tangentZ);
    gerstner(pos.xz, vec2(0.85, 0.50),  0.07,  3.2, 1.35, 0.30, pos, tangentX, tangentZ);

    pos.y += uWaterSurfaceLevel;
    vWave = pos.y - uWaterSurfaceLevel;
    vNormal = normalize(cross(tangentZ, tangentX));
    vSteepness = 1.0 - vNormal.y;
    vWorldPos = pos;
    gl_Position = uViewProj * vec4(pos, 1.0);
}
