#version 460

#include "common.glsl"

void main()
{
    vec2 st = uv * 2.0 - 1.0;
    
    float t = pc.shaderData.data.time * 0.4;
    
    for(float i = 1.0; i < 4.0; i++) {
        st.x += sin(st.y + t + i * 1.5) * 0.4 / i;
        st.y += cos(st.x + t + i * 2.0) * 0.3 / i;
    }
    
    float gridX = abs(sin(st.x * 4.0));
    float gridY = abs(cos(st.y * 4.0));
    float lines = 0.005 / (gridX * gridY);
    
    vec3 color1 = vec3(0.1, 0.4, 0.9); // Electric Blue
    vec3 color2 = vec3(0.8, 0.1, 0.6); // Deep Magenta
    vec3 color3 = vec3(0.1, 0.9, 0.6); // Cyan/Teal
    
    vec3 finalColor = mix(color1, color2, sin(st.x + t) * 0.5 + 0.5);
    finalColor = mix(finalColor, color3, cos(st.y - t) * 0.5 + 0.5);
    
    finalColor *= lines;
    
    float vignette = 1.0 - dot(uv - 0.5, uv - 0.5) * 1.5;
    finalColor *= max(vignette, 0.0);
    
    finalColor = vec3(1.0) - exp(-finalColor * 1.5);
    
    outColor = vec4(finalColor, 1.0);
}