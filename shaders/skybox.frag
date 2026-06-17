#version 330 core
in vec3 vTexCoord;

uniform samplerCube uCubemap;

out vec4 FragColor;

void main() {
    vec3 dir = normalize(vTexCoord);
    vec3 col = texture(uCubemap, dir).rgb;
    float depthHint = clamp(-dir.y * 0.5 + 0.5, 0.0, 1.0);
    col = mix(col, vec3(0.01, 0.04, 0.08), depthHint * 0.35);
    FragColor = vec4(col, 1.0);
}
