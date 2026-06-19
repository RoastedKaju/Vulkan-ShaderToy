#version 460

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

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main()
{
    // Center and scale UVs (assuming uv is 0.0 to 1.0)
    vec2 st = uv * 2.0 - 1.0;
    
    float t = pc.shaderData.data.time * 0.4;
    
    // Iterative domain warping (layering distorted coordinates)
    for(float i = 1.0; i < 4.0; i++) {
        st.x += sin(st.y + t + i * 1.5) * 0.4 / i;
        st.y += cos(st.x + t + i * 2.0) * 0.3 / i;
    }
    
    // Create a sharp geometric neon grid structure based on the warped space
    float gridX = abs(sin(st.x * 4.0));
    float gridY = abs(cos(st.y * 4.0));
    float lines = 0.005 / (gridX * gridY); // Intense neon glow lines
    
    // Dynamic psychedelic color palette shifts
    vec3 color1 = vec3(0.1, 0.4, 0.9); // Electric Blue
    vec3 color2 = vec3(0.8, 0.1, 0.6); // Deep Magenta
    vec3 color3 = vec3(0.1, 0.9, 0.6); // Cyan/Teal
    
    // Mix colors based on the warped coordinates and time
    vec3 finalColor = mix(color1, color2, sin(st.x + t) * 0.5 + 0.5);
    finalColor = mix(finalColor, color3, cos(st.y - t) * 0.5 + 0.5);
    
    // Apply the glowing line mask
    finalColor *= lines;
    
    // Add a dark vignette/depth fade towards the screen edges
    float vignette = 1.0 - dot(uv - 0.5, uv - 0.5) * 1.5;
    finalColor *= max(vignette, 0.0);
    
    // Tonemapping/Gamma Correction
    finalColor = vec3(1.0) - exp(-finalColor * 1.5);
    
    outColor = vec4(finalColor, 1.0);
}