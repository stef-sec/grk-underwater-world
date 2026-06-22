#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

uniform float uWaterLevel;
uniform float uFogDensity;
uniform vec3 uLightDir;
uniform vec3 uDeepColor;
uniform vec3 uColor;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    float diffuse = max(dot(N, L), 0.0);
    float depthFactor = clamp((uWaterLevel - vWorldPos.y) / 22.0, 0.0, 1.0);
    float fog = 1.0 - exp(-uFogDensity * depthFactor * depthFactor * 18.0);
    vec3 lit = uColor * (0.22 + diffuse * 0.78);
    lit = mix(lit, uDeepColor, fog * 0.85);
    lit = mix(lit, vec3(0.04, 0.14, 0.20), depthFactor * 0.35);
    FragColor = vec4(lit, 1.0);
}
