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
    float moonSpec = pow(max(dot(N, H), 0.0), 48.0);

    vec3 toSpot = uSpotPos - vWorldPos;
    float spotDist = length(toSpot);
    vec3 spotL = normalize(toSpot);
    float theta = dot(normalize(-toSpot), normalize(uSpotDir));
    float cone = clamp((theta - uSpotOuter) / max(uSpotInner - uSpotOuter, 0.0001), 0.0, 1.0);
    float attenuation = 1.0 / (1.0 + 0.06 * spotDist + 0.025 * spotDist * spotDist);
    float spotTerm = clamp(cone * attenuation * uSpotIntensity, 0.0, 3.5);
    float spotNdotL = max(dot(N, spotL), 0.0);
    float spotBoost = spotTerm * spotNdotL;

    float waterDepth = clamp((uWaterLevel - vWorldPos.y) / 26.0, 0.0, 1.0);
    float dist = length(uCameraPos - vWorldPos);
    float fog = 1.0 - exp(-uFogDensity * max(dist * 0.10, waterDepth * waterDepth * 14.0));

    vec3 lit = uBaseColor * (vec3(0.12) + uLightColor * moonDiffuse * 0.65);
    lit += uLightColor * moonSpec * 0.15;
    // Tinted torch highlight — keeps orange/gold fish color instead of white washout.
    lit += uBaseColor * mix(vec3(0.08), uSpotColor * 0.35, clamp(spotBoost * 0.14, 0.0, 0.75));
    lit = mix(lit, vec3(0.02, 0.08, 0.14), waterDepth * 0.30);
    lit = mix(lit, uDeepColor, fog * 0.82);

    FragColor = vec4(lit * uExposure, 1.0);
}
