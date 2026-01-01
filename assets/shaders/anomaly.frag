#version 450

layout(set = 0, binding = 1) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time; // x = total_time
    vec4 pad1;
    vec4 pad2;
} global_u;

layout(location = 0) in vec4 inFragPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outFragColor;

// --- Helper: Spatial Noise (Matches your Vertex Shader logic) ---
float get_density(vec3 pos) {
    float t = global_u.time.x * 0.0005;
    vec3 p = pos * 0.2;
    float n = sin(p.x + t) * cos(p.y + t * 0.7) * sin(p.z + t * 0.5);
    return clamp(n * 1.5 + 0.3, 0.0, 1.0);
}

vec3 get_palette(float t) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.00, 0.33, 0.67);
    return a + b * cos(6.28318 * (c * t + d));
}

void main() {
    vec3 N = normalize(inNormal.xyz);
    vec3 pos = inFragPos.xyz;
    vec3 L_main = normalize(global_u.light_pos.xyz - pos);

    // 1. Calculate Local Density
    float density = get_density(pos);

    // 2. Point Lights from Density "Peaks"
    // We raise density to a high power to find the "hottest" centers of the clumps
    float lightIntensity = pow(density, 8.0) * 15.0;
    vec3 lightColor = get_palette(density + 0.2); // Shift palette slightly for light color

    // 3. Hemisphere Ambient (Lower Bound)
    vec3 skyColor = vec3(0.1, 0.15, 0.25);
    vec3 groundColor = vec3(0.05, 0.02, 0.02);
    float hemis = dot(N, vec3(0, 1, 0)) * 0.5 + 0.5;
    vec3 ambient = mix(groundColor, skyColor, hemis);

    // 4. Diffuse (Main Light + Density Light)
    // Main Sun/World light
    float diff = max(dot(N, L_main), 0.0) * 0.8;

    // Local Point Light (Omni-directional glow from the clump center)
    // Since we are inside the clump, we simulate this as an emissive/wrap-around light
    float localLight = clamp(dot(N, -normalize(pos)) * 0.5 + 0.5, 0.0, 1.0);
    vec3 densityLightTerm = lightColor * localLight * lightIntensity;

    // 5. Albedo / Palette
    float color_t = length(pos) * 0.05 + global_u.time.x * 0.0001;
    vec3 albedo = get_palette(color_t + inColor.r);

    // 6. Anti-Aliasing (Fwidth Edge Darkening)
    vec3 V = normalize(-pos);
    float edge = fwidth(dot(N, V));
    float edgeMask = 1.0 - smoothstep(0.0, 0.15, edge);

    // 7. Final Composite
    // We add the density light as both a surface light and an emissive glow
    vec3 surfaceLighting = (ambient + (diff * 1.0)) * albedo;
    vec3 finalColor = surfaceLighting + (densityLightTerm * albedo * 0.5);

    // Add "Core" glow for the densest areas
    finalColor += lightColor * pow(density, 12.0) * 2.0;

    finalColor *= edgeMask;

    // Distance Fog
    float dist = length(pos);
    float fog = exp(-dist * 0.015);
    finalColor = mix(vec3(0.01, 0.01, 0.02), finalColor, fog);

    outFragColor = vec4(finalColor, 1.0);
}