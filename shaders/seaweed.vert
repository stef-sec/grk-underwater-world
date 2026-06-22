#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uViewProj;
uniform mat4 uModel;
uniform float uTime;

out vec3 vWorldPos;
out vec3 vNormal;

void main() {
    vec3 pos = aPos;
    float sway = sin(uTime * 1.3 + pos.y * 2.0 + pos.x * 0.5) * 0.07 * pos.y;
    pos.x += sway;
    pos.z += cos(uTime * 1.1 + pos.y * 1.7) * 0.04 * pos.y;
    vec4 world = uModel * vec4(pos, 1.0);
    vWorldPos = world.xyz;
    vNormal = mat3(uModel) * aNormal;
    gl_Position = uViewProj * world;
}
