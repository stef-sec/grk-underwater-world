#version 330 core
in vec2 vUV;

uniform mat4 uInvViewProj;
uniform vec3 uCameraPos;
uniform float uWaterLevel;
uniform float uFogDensity;
uniform float uStrength;
uniform float uTime;
uniform vec3 uMoonDir;
uniform vec3 uMoonColor;
uniform vec3 uSpotPos;
uniform vec3 uSpotDir;
uniform vec3 uSpotColor;
uniform float uSpotInner;
uniform float uSpotOuter;
uniform float uSpotIntensity;
uniform sampler2D uSceneDepth;

out vec4 FragColor;

const int STEPS = 48;

vec3 worldPosOnRay(vec2 ndc, float ndcZ) {
    vec4 clip = vec4(ndc, ndcZ, 1.0);
    vec4 world = uInvViewProj * clip;
    return world.xyz / world.w;
}

float hash31(vec3 p) {
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p += dot(p, p.yzx + 19.19);
    return fract((p.x + p.y) * p.z);
}

float spotConeVolume(vec3 p) {
    if (uSpotIntensity <= 0.0) return 0.0;
    vec3 D = normalize(uSpotDir);
    vec3 rel = p - uSpotPos;
    float along = dot(rel, D);
    if (along < 1.5 || along > 40.0) return 0.0;

    vec3 axisPoint = uSpotPos + D * along;
    float radial = length(p - axisPoint);
    float outerAngle = acos(clamp(uSpotOuter, -1.0, 1.0));
    float coneRadius = max(along * tan(outerAngle), 0.05);
    float radialMask = 1.0 - smoothstep(coneRadius * 0.35, coneRadius, radial);
    radialMask = pow(radialMask, 1.35);

    float distAtten = 1.0 / (1.0 + 0.04 * along + 0.0015 * along * along);
    return radialMask * distAtten;
}

float underwaterMask(vec3 p) {
    return 1.0 - smoothstep(uWaterLevel - 0.05, uWaterLevel + 0.3, p.y);
}

float mediaDensity(vec3 p) {
    float underwater = underwaterMask(p);
    if (underwater < 0.01) return 0.0;

    float depth = clamp((uWaterLevel - p.y) / 20.0, 0.0, 1.0);
    float base = uFogDensity * (0.14 + depth * depth * 0.55);
    float n = hash31(p * 0.24 + vec3(uTime * 0.05));
    base *= 0.72 + 0.28 * n;

    float beam = spotConeVolume(p);
    base += beam * 0.08;

    return base * underwater;
}

float moonShaft(vec3 p, vec3 rayDir) {
    vec3 L = normalize(uMoonDir);
    float rayAlign = pow(max(dot(rayDir, L), 0.0), 5.0);
    float surfaceBand = smoothstep(uWaterLevel - 18.0, uWaterLevel - 0.4, p.y);
    float depthFade = 1.0 - smoothstep(uWaterLevel - 30.0, uWaterLevel - 2.0, p.y);
    vec2 drift = vec2(0.08, -0.05) * uTime;
    float stripes = sin((p.x + p.z * 0.55) * 0.42 + drift.x) * 0.5 + 0.5;
    stripes *= sin((p.x * -0.35 + p.z) * 0.31 + drift.y + 1.7) * 0.5 + 0.5;
    float shaftMask = smoothstep(0.30, 0.78, stripes);
    return surfaceBand * depthFade * (rayAlign * 0.55 + shaftMask * 0.75);
}

vec3 toneMap(vec3 c) {
    return vec3(1.0) - exp(-c * 0.9);
}

void main() {
    vec2 ndc = vUV * 2.0 - 1.0;
    ndc.y = -ndc.y;

    vec3 rayNear = worldPosOnRay(ndc, -1.0);
    vec3 rayFar = worldPosOnRay(ndc, 1.0);
    vec3 rayDir = normalize(rayFar - rayNear);

    float sceneDepth = texture(uSceneDepth, vUV).r;
    float maxDist = 60.0;
    if (sceneDepth < 0.9999) {
        vec3 sceneHit = worldPosOnRay(ndc, sceneDepth * 2.0 - 1.0);
        maxDist = min(maxDist, max(length(sceneHit - uCameraPos) - 0.35, 0.0));
    }
    if (maxDist <= 0.05) {
        FragColor = vec4(0.0);
        return;
    }

    vec3 accum = vec3(0.0);
    float transmittance = 1.0;
    float stepLen = maxDist / float(STEPS);
    float strength = uStrength * 0.30;
    float jitter = hash31(vec3(vUV * 173.31, uTime * 0.17));

    for (int i = 0; i < STEPS; ++i) {
        float t = (float(i) + jitter) * stepLen;
        vec3 p = uCameraPos + rayDir * t;

        float density = mediaDensity(p);
        float beam = spotConeVolume(p);
        if (density <= 0.00001 && beam <= 0.00001) continue;

        float moon = moonShaft(p, rayDir) * density;
        vec3 moonLit = uMoonColor * moon * 4.5;

        vec3 spotL = normalize(uSpotPos - p);
        float spotPhase = pow(max(dot(-rayDir, spotL), 0.0), 6.0);
        float softGlow = beam * (0.18 + spotPhase * 0.82);
        vec3 spotLit = uSpotColor * density * softGlow * 1.15;

        vec3 sampleLight = moonLit + spotLit;
        sampleLight = min(sampleLight, vec3(0.24));

        accum += transmittance * sampleLight * stepLen * strength;
        transmittance *= exp(-max(density, beam * 0.04) * stepLen * 0.7);
        if (transmittance < 0.02) break;
    }

    accum = toneMap(accum);
    float alpha = clamp(length(accum) * 1.0, 0.0, 0.88);
    FragColor = vec4(accum, alpha);
}
