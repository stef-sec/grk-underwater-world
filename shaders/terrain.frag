#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vPosLightSpace;

uniform float uTime;
uniform float uWaterLevel;
uniform float uFogDensity;
uniform vec3 uCameraPos;
uniform vec3 uBaseColor;
uniform vec3 uDeepColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;
uniform sampler2D uShadowMap;

out vec4 FragColor;

float shadowPCF(vec3 normal) {
    vec3 projCoords = vPosLightSpace.xyz / vPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 1.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) return 1.0;

    vec3 L = normalize(uLightDir);
    float bias = max(0.0018 * (1.0 - dot(normalize(normal), L)), 0.0006);
    float current = projCoords.z - bias;

    float shadow = 0.0;
    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closest = texture(uShadowMap, projCoords.xy + vec2(x, y) * texel).r;
            shadow += current > closest ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    float shadow = shadowPCF(N);
    float diffuse = max(dot(N, L), 0.0) * shadow;

    float heightT = clamp((vWorldPos.y + 11.0) / 9.0, 0.0, 1.0);
    float slope = 1.0 - clamp(N.y, 0.0, 1.0);
    vec3 mud = vec3(0.16, 0.14, 0.10);
    vec3 sand = uBaseColor;
    vec3 rock = vec3(0.24, 0.23, 0.22);
    vec3 albedo = mix(mud, sand, smoothstep(0.15, 0.75, heightT));
    albedo = mix(albedo, rock, smoothstep(0.18, 0.55, slope));

    float waterDepth = clamp((uWaterLevel - vWorldPos.y) / 22.0, 0.0, 1.0);
    float dist = length(uCameraPos - vWorldPos);
    float viewFog = 1.0 - exp(-uFogDensity * dist * 0.12);
    float depthFog = 1.0 - exp(-uFogDensity * waterDepth * waterDepth * 18.0);
    float fog = clamp(max(viewFog, depthFog), 0.0, 1.0);

    vec3 waterScatter = mix(vec3(0.04, 0.14, 0.22), vec3(0.01, 0.05, 0.10), waterDepth);
    vec3 lit = albedo * (uAmbient + uLightColor * diffuse * 0.95);
    lit = mix(lit, uDeepColor, fog * 0.9);
    lit = mix(lit, waterScatter, clamp(waterDepth * 0.55, 0.0, 1.0));

    float foam = smoothstep(0.42, 0.88, heightT + max(vWorldPos.y - (-7.5), 0.0) * 0.03);
    lit += foam * vec3(0.04, 0.07, 0.08);

    vec2 causticUV = vWorldPos.xz - L.xz * vWorldPos.y * 0.22;
    float caustic = 0.5 + 0.5 * sin(causticUV.x * 1.1 + uTime * 1.6)
                          * sin(causticUV.y * 0.95 - uTime * 1.2);
    lit += caustic * (1.0 - waterDepth) * (1.0 - fog) * shadow * 0.04;

    FragColor = vec4(lit, 1.0);
}
