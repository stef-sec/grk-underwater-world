#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uViewProj;
uniform mat4 uLightVP;

out vec3 vWorldPos;
out vec3 vNormal;
out vec4 vPosLightSpace;

void main() {
    vWorldPos = aPos;
    vNormal = normalize(aNormal);
    vPosLightSpace = uLightVP * vec4(aPos, 1.0);
    gl_Position = uViewProj * vec4(aPos, 1.0);
}
