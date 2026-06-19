#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct ShaderData
{
    float time;
    float deltaTime;
    uint frameCounter;
};

layout(buffer_reference, scalar) readonly buffer ShaderDataBuffer
{
    ShaderData data;
};

layout(push_constant) uniform PushConstants
{
    ShaderDataBuffer shaderData;
} pc;