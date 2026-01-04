#version 450

layout(location = 0) in vec4 inPosition; // world position
layout(location = 1) in vec4 inNormal;   // world normal
layout(location = 2) in vec4 inColor;    // albedo / material

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;

void main()
{
    outPosition = inPosition;

    vec3 normal = normalize(inNormal.xyz);
    outNormal = vec4(normal * 0.5 + 0.5, 1.0);  // Pack from [-1,1] to [0,1]

    outColor = inColor;
}
