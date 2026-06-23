#version 330 core
in float vWave;
in vec3 vNormal;
in vec3 vWorldPos;
in float vSteepness;

uniform float uWaterCameraY;
uniform float uWaterSurfaceLevel;
uniform vec3 uCameraPos;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uSpotPos;
uniform vec3 uSpotDir;
uniform vec3 uSpotColor;
uniform float uSpotInner;
uniform float uSpotOuter;
uniform float uSpotIntensity;
uniform float uTime;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);
    vec3 R = reflect(-L, N);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float fresnel = pow(1.0 - NdotV, 3.2);

    float diffuse = NdotL * 0.45 + 0.04;
    float specMoon = pow(max(dot(R, V), 0.0), 140.0);
    float specBroad = pow(max(dot(N, H), 0.0), 36.0);

    float crest = smoothstep(0.06, 0.32, vWave);
    float trough = smoothstep(-0.28, -0.06, -vWave);
    float foam = smoothstep(0.22, 0.55, vSteepness) * smoothstep(0.08, 0.25, vWave);

    vec3 deepWater = vec3(0.02, 0.08, 0.14);
    vec3 midWater = vec3(0.04, 0.16, 0.24);
    vec3 shallow = vec3(0.08, 0.28, 0.36);
    vec3 moonTint = uLightColor * 0.35;

    vec3 color = mix(deepWater, midWater, crest * 0.7 + diffuse * 0.3);
    color = mix(color, shallow, crest * 0.55);
    color = mix(color, moonTint, fresnel * 0.5);
    color += uLightColor * specMoon * 0.45;
    color += specBroad * 0.08;
    color += foam * vec3(0.08, 0.10, 0.12);
    color = mix(color, deepWater * 1.2, trough * 0.35);

    vec3 lightToFrag = vWorldPos - uSpotPos;
    float dist = length(lightToFrag);
    vec3 spotDir = normalize(lightToFrag);
    float theta = dot(spotDir, normalize(uSpotDir));
    float cone = clamp((theta - uSpotOuter) / max(uSpotInner - uSpotOuter, 0.0001), 0.0, 1.0);
    float attenuation = 1.0 / (1.0 + 0.045 * dist + 0.02 * dist * dist);
    float spot = cone * attenuation * uSpotIntensity;
    float spotSpec = pow(max(dot(reflect(-normalize(uSpotPos - vWorldPos), N), V), 0.0), 90.0);
    color += uSpotColor * (spot * 0.18 + spotSpec * spot * 0.6);

    float alpha = mix(0.38, 0.55, fresnel);
    if (uWaterCameraY < uWaterSurfaceLevel) {
        color = mix(color, vec3(0.03, 0.12, 0.20), 0.22);
        alpha = mix(0.46, 0.62, fresnel);
    }

    FragColor = vec4(color, alpha);
}
