#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in float aFoam;
layout(location = 5) in float aSlope;

uniform mat4 uViewProj;
uniform mat4 uLightVP;
uniform float uTime;

out vec3 vWorldPos;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vNormalWs;
out vec3 vGeomNormal;
out float vFoam;
out float vSlope;
out vec4 vPosLightSpace;

void main() {
    vec3 pos = aPos;
    pos.y += sin(pos.x * 0.22 + uTime * 0.7) * 0.12 + cos(pos.z * 0.19 - uTime * 0.5) * 0.10;

    vec3 T = normalize(aTangent);
    vec3 B = normalize(aBitangent);
    vec3 N = normalize(aNormal + vec3(
        cos(pos.x * 0.22 + uTime * 0.7) * 0.03,
        0.0,
        -sin(pos.z * 0.19 - uTime * 0.5) * 0.03
    ));
    T = normalize(cross(N, B));
    B = normalize(cross(T, N));

    vWorldPos = pos;
    vTangent = T;
    vBitangent = B;
    vNormalWs = N;
    vGeomNormal = normalize(aNormal);
    vFoam = aFoam;
    vSlope = aSlope;
    vPosLightSpace = uLightVP * vec4(pos, 1.0);
    gl_Position = uViewProj * vec4(pos, 1.0);
}
