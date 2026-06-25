#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uViewProj;
uniform vec3 uColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(uColor, 1.0);
}
