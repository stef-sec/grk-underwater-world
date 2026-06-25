#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aSeed;
layout(location = 2) in float aSize;

uniform mat4 uViewProj;
uniform vec3 uCameraPos;
uniform float uWaterLevel;
uniform float uTime;

out float vAlpha;
out float vSeed;

void main() {
    vec3 pos = aPos;
    pos.x += sin(uTime * 0.17 + aSeed * 31.0 + pos.z * 0.07) * 0.42;
    pos.y += sin(uTime * 0.11 + aSeed * 19.0) * 0.22;
    pos.z += cos(uTime * 0.13 + aSeed * 23.0 + pos.x * 0.05) * 0.36;

    vec3 toCamera = uCameraPos - pos;
    float dist = length(toCamera);
    float underwater = 1.0 - smoothstep(uWaterLevel - 0.05, uWaterLevel + 0.35, pos.y);
    float distanceFade = 1.0 - smoothstep(7.0, 38.0, dist);
    vAlpha = underwater * distanceFade * (0.22 + aSeed * 0.35);
    vSeed = aSeed;

    gl_PointSize = aSize * (1.0 - smoothstep(2.0, 44.0, dist));
    gl_Position = uViewProj * vec4(pos, 1.0);
}
