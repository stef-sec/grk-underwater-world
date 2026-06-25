#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

uniform vec3 uScreenSize;

out vec4 vColor;

void main() {
    vec2 clip;
    clip.x = aPos.x / uScreenSize.x * 2.0 - 1.0;
    clip.y = 1.0 - aPos.y / uScreenSize.y * 2.0;
    gl_Position = vec4(clip, 0.0, 1.0);
    vColor = aColor;
}
