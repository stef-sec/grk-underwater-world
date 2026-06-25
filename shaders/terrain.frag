#version 330 core
in vec3 vWorldPos;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vNormalWs;
in vec3 vGeomNormal;
in float vFoam;
in float vSlope;
in vec4 vPosLightSpace;

uniform float uTime;
uniform float uWaterLevel;
uniform float uFogDensity;
uniform vec3 uCameraPos;
uniform vec3 uDeepColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;
uniform vec3 uSpotPos;
uniform vec3 uSpotDir;
uniform vec3 uSpotColor;
uniform float uSpotInner;
uniform float uSpotOuter;
uniform float uSpotIntensity;
uniform vec3 uSandBaseColor;
uniform float uSandMetallic;
uniform float uSandRoughness;
uniform vec3 uRockBaseColor;
uniform float uRockMetallic;
uniform float uRockRoughness;
uniform float uNormalStrength;
uniform float uDebugMaterials;
uniform float uExposure;
uniform sampler2D uShadowMap;
uniform sampler2D uSandNormalMap;
uniform sampler2D uRockNormalMap;

out vec4 FragColor;

const float PI = 3.14159265;

float shadowPCF(vec3 geomNormal) {
    vec3 projCoords = vPosLightSpace.xyz / vPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 1.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) return 1.0;

    vec3 L = normalize(uLightDir);
    vec3 N = normalize(geomNormal);
    float bias = max(0.0035 * (1.0 - dot(N, L)), 0.0012);
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

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotX, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotX / (NdotX * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float ggx1 = GeometrySchlickGGX(max(dot(N, V), 0.0), roughness);
    float ggx2 = GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 sampleNormalMapTS(sampler2D normalMap, vec2 uv) {
    vec3 n = texture(normalMap, uv).xyz * 2.0 - 1.0;
    return normalize(n);
}

vec3 evaluatePBR(vec3 N, vec3 V, vec3 L, vec3 baseColor, float metallic, float roughness, float shadow) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 specular = (NDF * G * F) / max(4.0 * NdotV * NdotL, 0.0001);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    vec3 diffuse = kD * baseColor / PI;
    return (diffuse + specular) * NdotL * shadow;
}

float spotFalloff(vec3 worldPos, vec3 lightDir) {
    vec3 lightToFrag = normalize(worldPos - uSpotPos);
    float theta = dot(lightToFrag, normalize(lightDir));
    float cone = clamp((theta - uSpotOuter) / max(uSpotInner - uSpotOuter, 0.0001), 0.0, 1.0);
    cone *= cone;
    float dist = length(worldPos - uSpotPos);
    float attenuation = 1.0 / (1.0 + 0.06 * dist + 0.018 * dist * dist);
    return cone * attenuation * uSpotIntensity;
}

float causticLayer(vec2 uv, float t, float scale, vec2 drift) {
    vec2 p = uv * scale + drift * t;
    return 0.5 + 0.5 * sin(p.x * 2.7 + sin(p.y * 1.9)) + 0.35 * sin(p.y * 3.1 - t * 0.6);
}

void main() {
    float sandMask = smoothstep(0.22, 0.78, vFoam);
    sandMask *= 1.0 - smoothstep(0.10, 0.42, vSlope);

    if (uDebugMaterials > 0.5) {
        FragColor = vec4(mix(vec3(0.85, 0.15, 0.10), vec3(0.95, 0.85, 0.35), sandMask), 1.0);
        return;
    }

    mat3 tbn = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormalWs));
    vec2 uv = vWorldPos.xz;
    vec3 sandNts = sampleNormalMapTS(uSandNormalMap, uv * 0.18);
    vec3 rockNts = sampleNormalMapTS(uRockNormalMap, uv * 0.30);
    vec3 Nts = normalize(mix(rockNts, sandNts, sandMask));
    Nts = normalize(vec3(Nts.xy * uNormalStrength, max(Nts.z, 0.15)));
    vec3 N = normalize(tbn * Nts);

    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 L = normalize(uLightDir);
    float shadow = shadowPCF(vGeomNormal);

    vec3 sandLit = evaluatePBR(N, V, L, uSandBaseColor, uSandMetallic, uSandRoughness, shadow);
    vec3 rockLit = evaluatePBR(N, V, L, uRockBaseColor, uRockMetallic, uRockRoughness, shadow);
    vec3 lit = mix(rockLit, sandLit, sandMask);
    lit += uAmbient * mix(uRockBaseColor, uSandBaseColor, sandMask);
    lit *= mix(0.85, 1.0, dot(uLightColor, vec3(0.299, 0.587, 0.114)));

    vec3 baseMix = mix(uRockBaseColor, uSandBaseColor, sandMask);
    float metallicMix = mix(uRockMetallic, uSandMetallic, sandMask);
    float roughnessMix = mix(uRockRoughness, uSandRoughness, sandMask);

    float spotCone = spotFalloff(vWorldPos, uSpotDir);
    vec3 spotL = normalize(uSpotPos - vWorldPos);
    float flatness = smoothstep(0.45, 0.88, vGeomNormal.y);
    float spotNdotL = max(dot(N, spotL), 0.0);
    float geomNdotL = max(dot(vGeomNormal, spotL), 0.0);
    // Flat seabed faces the beam poorly in classic Lambert terms; keep cone lighting there.
    float spotReceiver = mix(max(spotNdotL, geomNdotL), max(max(spotNdotL, geomNdotL), 0.42), flatness);

    vec3 spotPbr = evaluatePBR(N, V, spotL, baseMix, metallicMix, roughnessMix, 1.0);
    vec3 spotDiffuse = baseMix * spotReceiver * spotCone * 0.10;
    vec3 spotSpecular = spotPbr * spotCone;
    vec3 spotContribution = uSpotColor * (spotDiffuse + spotSpecular * 0.12);

    lit += spotContribution;

    float waterDepth = clamp((uWaterLevel - vWorldPos.y) / 22.0, 0.0, 1.0);
    float dist = length(uCameraPos - vWorldPos);
    float viewFog = 1.0 - exp(-uFogDensity * dist * 0.12);
    float depthFog = 1.0 - exp(-uFogDensity * waterDepth * waterDepth * 18.0);
    float fog = clamp(max(viewFog, depthFog), 0.0, 1.0);

    vec3 waterScatter = mix(vec3(0.04, 0.14, 0.22), vec3(0.01, 0.05, 0.10), waterDepth);
    lit = mix(lit, uDeepColor, fog * 0.9);
    lit = mix(lit, waterScatter, clamp(waterDepth * 0.55, 0.0, 1.0));

    vec2 causticUV = vWorldPos.xz - L.xz * vWorldPos.y * 0.18;
    float caustic = causticLayer(causticUV, uTime, 0.55, vec2(0.08, 0.06));
    caustic = smoothstep(0.85, 1.35, caustic);
    lit += caustic * (1.0 - waterDepth) * (1.0 - fog) * shadow * 0.022;

    FragColor = vec4(lit * uExposure, 1.0);
}
