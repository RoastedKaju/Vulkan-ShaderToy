#version 460

#include "common.glsl"

void main()
{
    float time = pc.shaderData.data.time;
    vec2 adjusted_uv = (uv - vec2(0.7)) * 3.0;

    float e = 0.0;

    for (float i = 3.0; i <= 15.0; i += 1.0) 
    {
        e += 0.007 / abs((i / 15.0) + sin((time / 2.0) + 0.15 * i * (adjusted_uv.x) * (cos(i / 4.0 + (time / 2.0) + adjusted_uv.x * 2.2))) + 2.5 * adjusted_uv.y);
    }
    outColor = vec4(vec3(e / 1.6, e / 11.6, e / 1.6), 1.0); 
}