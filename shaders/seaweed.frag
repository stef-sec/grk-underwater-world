#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

uniform float uWaterLevel;
uniform float uFogDensity;
uniform vec3 uLightDir;
uniform vec3 uDeepColor;
uniform vec3 uColor;
uniform vec3 uSpotPos;
uniform vec3 uSpotDir;
uniform vec3 uSpotColor;
uniform float uSpotInner;
uniform float uSpotOuter;
uniform float uSpotIntensity;
uniform float uExposure;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    float diffuse = max(dot(N, L), 0.0);
    vec3 toLight = uSpotPos - vWorldPos;
    vec3 spotL = normalize(toLight);
    float theta = dot(normalize(-toLight), normalize(uSpotDir));
    float cone = clamp((theta - uSpotOuter) / max(uSpotInner - uSpotOuter, 0.0001), 0.0, 1.0);
    float spotDist = length(uSpotPos - vWorldPos);
    float attenuation = 1.0 / (1.0 + 0.05 * spotDist + 0.018 * spotDist * spotDist);
    float spot = max(dot(N, spotL), 0.0) * cone * attenuation * uSpotIntensity;
    float depthFactor = clamp((uWaterLevel - vWorldPos.y) / 22.0, 0.0, 1.0);
    float fog = 1.0 - exp(-uFogDensity * depthFactor * depthFactor * 18.0);
    vec3 lit = uColor * (0.03 + diffuse * 0.35);
    lit += uSpotColor * spot * 0.12;
    lit = mix(lit, uDeepColor, fog * 0.92);
    lit = mix(lit, vec3(0.04, 0.14, 0.20), depthFactor * 0.35);
    FragColor = vec4(lit * uExposure, 1.0);
}
