#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uLightVP;
uniform float uTime;

void main() {
    vec3 pos = aPos;
    pos.y += sin(pos.x * 0.22 + uTime * 0.7) * 0.12 + cos(pos.z * 0.19 - uTime * 0.5) * 0.10;
    gl_Position = uLightVP * vec4(pos, 1.0);
}
