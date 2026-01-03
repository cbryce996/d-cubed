#version 450

layout(set = 0, binding = 0) uniform ViewUniform {
    mat4 view_projection;
} view_u;

layout(set = 0, binding = 1) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time;
    vec4 camera_pos;
    vec4 pad2;
} global_u;

layout(location = 0) in vec4 inMeshPos;
layout(location = 1) in vec4 inMeshNormal;
layout(location = 2) in vec4 inMeshColor;
layout(location = 3) in vec4 inMeshPad;

layout(location = 4) in vec4 inInstPos;
layout(location = 5) in vec4 inInstRot;
layout(location = 6) in vec4 inInstScale;
layout(location = 7) in vec4 inInstPad;

layout(location = 0) out vec4 outFragPos;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;
layout(location = 3) out float outRadius;
layout(location = 4) out float cubeSeed;

float hash11(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

vec3 hash31(float p) {
    return vec3(
    hash11(p + 1.0),
    hash11(p + 2.0),
    hash11(p + 3.0)
    );
}

mat3 rodriguesRotation(vec3 axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;

    axis = normalize(axis);

    return mat3(
        t*axis.x*axis.x + c,        t*axis.x*axis.y - s*axis.z, t*axis.x*axis.z + s*axis.y,
        t*axis.x*axis.y + s*axis.z, t*axis.y*axis.y + c,        t*axis.y*axis.z - s*axis.x,
        t*axis.x*axis.z - s*axis.y, t*axis.y*axis.z + s*axis.x, t*axis.z*axis.z + c
    );
}

mat3 quaternionRotation(vec3 axis, float angle) {
    axis = normalize(axis);

    float a = angle * 0.2;
    float s = sin(a);

    vec4 q = vec4(axis * s, cos(a));

    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;


    return mat3(
        1.0 - 2.0 * (yy + zz), 2.0 * (xy - wz),       2.0 * (xz + wy),
        2.0 * (xy + wz),       1.0 - 2.0 * (xx + zz), 2.0 * (yz - wx),
        2.0 * (xz - wy),       2.0 * (yz + wx),       1.0 - 2.0 * (xx + yy)
    );
}

float radialFallOff(
    vec3 position,
    vec3 center,
    float radius,
    float power
) {
    float distance = length(position - center);
    float normalized_distance = distance / radius; // use radius here!
    float inverted_distance = clamp(1.0 - normalized_distance, 0.0, 1.0); // optionally cap at 1.0
    return pow(inverted_distance, power);
}

float sinWave(
    float time,
    float phase,
    float frequency,
    float amplitude
) {
    return sin(phase * frequency + time) * amplitude;
}

vec3 sinWave(
    float time,
    vec3 phase,
    vec3 frequency,
    vec3 amplitude
) {
    return vec3(
        sin(phase.x * frequency.x + time) * amplitude.x,
        cos(phase.y * frequency.y + time) * amplitude.y,
        sin(phase.z * frequency.z + time) * amplitude.z
    );
}

vec3 computeFieldCenter(float time) {
    // Define pyramid vertices in local space
    vec3 apex    = vec3(0.0, 10.0, 0.0);
    vec3 base0   = vec3(-10.0, 0.0, -10.0);
    vec3 base1   = vec3(10.0, 0.0, -10.0);
    vec3 base2   = vec3(10.0, 0.0, 10.0);
    vec3 base3   = vec3(-10.0, 0.0, 10.0);

    // Cycle through the pyramid points
    float cycleSpeed = 0.05; // controls how fast it moves
    float t = fract(time * cycleSpeed); // [0,1] looping

    // Map t to 5 segments (apex -> base0 -> base1 -> base2 -> base3 -> apex)
    float segment = t * 5.0;
    int segIndex = int(floor(segment));
    float localT = segment - float(segIndex);

    // smoothstep for ease-in/out
    float smoothT = localT * localT * (3.0 - 2.0 * localT);

    vec3 p0, p1;
    if(segIndex == 0) { p0 = apex;  p1 = base0; }
    else if(segIndex == 1) { p0 = base0; p1 = base1; }
    else if(segIndex == 2) { p0 = base1; p1 = base2; }
    else if(segIndex == 3) { p0 = base2; p1 = base3; }
    else { p0 = base3; p1 = apex; }

    return mix(p0, p1, smoothT);
}


vec3 computeFieldPath(
    vec3 instancePos,
    float time,
    float field,
    float seed
) {
    vec3 rand = hash31(seed) * 2.0 - 1.0;

    float t = time + rand.x * 5.0;

    vec3 path;
    path.x = cos(t * (0.3 + rand.x)) * (2.5 + rand.y);
    path.y = sin(t * (0.9 + rand.y)) * (1.0 + rand.z);
    path.z = sin(t * (1.5 + rand.z)) * (1.0 + rand.x);

    return path * field;
}

float computeField(vec3 position, vec3 center) {
    float field = radialFallOff(position, center, 5.0, 0.8) ;
    return clamp(field, 0.0, 0.2);
}

vec3 transformMesh(
    vec3 position,
    vec3 scale,
    mat3 rotation
) {
    return rotation * (position * scale);
}

void main() {
    float time = global_u.time.x * 0.001;

    vec3 instancePos = inInstPos.xyz;
    vec3 axis = inInstRot.xyz;
    float phase = inInstRot.w;

    // --- Field ---
    vec3 fieldCenter = computeFieldCenter(time);
    float field = computeField(instancePos, fieldCenter);

    // --- Motion ---
    float seed = dot(instancePos, vec3(12.9898, 78.233, 45.164));

    vec3 path = computeFieldPath(
        instancePos,
        time,
        field,
        seed
    );

    // --- Transform ---
    mat3 rotation = quaternionRotation(axis, phase + time);
    vec3 scale = inInstScale.xyz * (field + 0.1);

    vec3 worldPos =
    instancePos +
    path +
    transformMesh(inMeshPos.xyz, scale, rotation);

    vec3 worldNormal = normalize(rotation * inMeshNormal.xyz);

    // --- GBuffer ---
    outFragPos = vec4(worldPos, 1.0);
    outNormal  = vec4(worldNormal, 0.0);
    outColor   = inMeshColor;
    outRadius  = 15.0;
    cubeSeed   = seed;

    gl_Position = view_u.view_projection * vec4(worldPos, 1.0);
}
