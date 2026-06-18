#version 460

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct ShaderData
{
    vec2 resolution;
    vec2 mouse;

    float time;
    float deltaTime;

    uint frameIndex;
    uint flags;
};

layout(buffer_reference, scalar) readonly buffer ShaderDataBuffer
{
    ShaderData data;
};

layout(push_constant) uniform PushConstants
{
    ShaderDataBuffer shaderData;
} pc;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 p = floor(uv * 10.0);
    float checker = mod(p.x + p.y, 2.0);
    vec3 color = mix(vec3(0.1, 0.1, 0.1), vec3(0.9, 0.9, 0.9), checker);
    outColor = vec4(color, 1.0);
}
