#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

uniform float uWaterLevel;
uniform float uFogDensity;
uniform vec3 uCameraPos;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uSpotPos;
uniform vec3 uSpotDir;
uniform vec3 uSpotColor;
uniform float uSpotInner;
uniform float uSpotOuter;
uniform float uSpotIntensity;
uniform vec3 uDeepColor;
uniform vec3 uBaseColor;
uniform float uExposure;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 moonL = normalize(uLightDir);
    vec3 H = normalize(moonL + V);

    float moonDiffuse = max(dot(N, moonL), 0.0);
    float moonSpec = pow(max(dot(N, H), 0.0), 64.0);

    vec3 toSpot = uSpotPos - vWorldPos;
    float spotDist = length(toSpot);
    vec3 spotL = normalize(toSpot);
    float theta = dot(normalize(-toSpot), normalize(uSpotDir));
    float cone = clamp((theta - uSpotOuter) / max(uSpotInner - uSpotOuter, 0.0001), 0.0, 1.0);
    float attenuation = 1.0 / (1.0 + 0.045 * spotDist + 0.018 * spotDist * spotDist);
    float spotTerm = cone * attenuation * uSpotIntensity;
    float spotDiffuse = max(dot(N, spotL), 0.0) * spotTerm;
    float spotSpec = pow(max(dot(N, normalize(spotL + V)), 0.0), 96.0) * spotTerm;

    float waterDepth = clamp((uWaterLevel - vWorldPos.y) / 26.0, 0.0, 1.0);
    float dist = length(uCameraPos - vWorldPos);
    float fog = 1.0 - exp(-uFogDensity * max(dist * 0.10, waterDepth * waterDepth * 14.0));

    vec3 lit = uBaseColor * (vec3(0.08) + uLightColor * moonDiffuse * 0.55);
    lit += uLightColor * moonSpec * 0.18;
    lit += uSpotColor * (spotDiffuse * 0.35 + spotSpec * 0.20);
    lit = mix(lit, vec3(0.018, 0.075, 0.135), waterDepth * 0.38);
    lit = mix(lit, uDeepColor, fog * 0.82);
    lit = mix(lit, vec3(lit.r * 0.72, lit.g * 0.96, lit.b * 1.12), waterDepth * 0.38);

    FragColor = vec4(lit * uExposure, 1.0);
}
