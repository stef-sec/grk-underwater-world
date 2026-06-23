#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in float aFoam;
layout(location = 5) in float aSlope;

uniform mat4 uViewProj;
uniform mat4 uLightVP;
out vec3 vWorldPos;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vNormalWs;
out vec3 vGeomNormal;
out float vFoam;
out float vSlope;
out vec4 vPosLightSpace;

void main() {
    vec3 T = normalize(aTangent);
    vec3 B = normalize(aBitangent);
    vec3 N = normalize(aNormal);
    T = normalize(cross(N, B));
    B = normalize(cross(T, N));

    vWorldPos = aPos;
    vTangent = T;
    vBitangent = B;
    vNormalWs = N;
    vGeomNormal = normalize(aNormal);
    vFoam = aFoam;
    vSlope = aSlope;
    vPosLightSpace = uLightVP * vec4(aPos, 1.0);
    gl_Position = uViewProj * vec4(aPos, 1.0);
}
