#version 330 core
in float vAlpha;
in float vSeed;

uniform vec3 uTint;

out vec4 FragColor;

void main() {
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(p, p);
    if (r2 > 1.0) discard;

    float soft = exp(-r2 * 2.8);
    float sparkle = 0.75 + 0.25 * sin(vSeed * 41.0);
    FragColor = vec4(uTint * sparkle, vAlpha * soft);
}
