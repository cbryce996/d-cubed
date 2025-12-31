#include <metal_stdlib>
using namespace metal;

/* ================================
   Uniforms
   ================================ */

struct SkyUniform {
    float4x4 inv_view_proj;
    float4   time;     // x = time
    float4   params;   // x = star density, y = nebula strength
};

/* ================================
   Vertex Output
   ================================ */

struct VSOut {
    float4 position [[position]];
    float3 ray;
};

/* ================================
   Fullscreen Triangle Vertex Shader
   ================================ */

vertex VSOut sky_vert(uint vid [[vertex_id]],
                      constant SkyUniform& u [[buffer(0)]])
{
    float2 pos;
    pos.x = (vid == 2) ? 3.0 : -1.0;
    pos.y = (vid == 1) ? 3.0 : -1.0;

    float4 clip = float4(pos, 0.0, 1.0);
    float4 world = u.inv_view_proj * clip;

    VSOut out;
    out.position = clip;
    out.ray = normalize(world.xyz / world.w);
    return out;
}

/* ================================
   Hash / Noise
   ================================ */

float hash(float3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(float3 p) {
    float3 i = floor(p);
    float3 f = fract(p);

    float n000 = hash(i + float3(0,0,0));
    float n100 = hash(i + float3(1,0,0));
    float n010 = hash(i + float3(0,1,0));
    float n110 = hash(i + float3(1,1,0));
    float n001 = hash(i + float3(0,0,1));
    float n101 = hash(i + float3(1,0,1));
    float n011 = hash(i + float3(0,1,1));
    float n111 = hash(i + float3(1,1,1));

    float3 u = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(mix(n000, n100, u.x), mix(n010, n110, u.x), u.y),
        mix(mix(n001, n101, u.x), mix(n011, n111, u.x), u.y),
        u.z
    );
}

float fbm(float3 p) {
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 5; i++) {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

/* ================================
   Fragment Shader
   ================================ */

fragment float4 sky_frag(VSOut in [[stage_in]],
                         constant SkyUniform& u [[buffer(0)]])
{
    float3 dir = normalize(in.ray);
    float t = u.time.x;

    /* -------- Stars -------- */

    float starHash = hash(dir * 8000.0);
    float star = step(1.0 - u.params.x, starHash);
    float starTwinkle = 0.7 + 0.3 * sin(t * 3.0 + starHash * 100.0);
    star *= starTwinkle;

    float3 starColor = float3(1.0, 1.0, 1.0) * star;

    /* -------- Nebula -------- */

    float3 nebulaDir = dir + float3(0.0, t * 0.01, 0.0);
    float nebula = fbm(nebulaDir * 3.0);

    nebula = smoothstep(0.3, 0.8, nebula);
    nebula *= u.params.y;

    float3 nebulaColor =
        mix(float3(0.1, 0.2, 0.5),
            float3(0.8, 0.4, 1.0),
            nebula);

    /* -------- Galaxy Band -------- */

    float galaxy = exp(-abs(dir.y) * 6.0);
    nebulaColor *= galaxy;

    /* -------- Final -------- */

    float3 color = nebulaColor + starColor;

    return float4(saturate(color), 1.0);
}
