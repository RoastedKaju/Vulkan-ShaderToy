#version 460

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 p = floor(uv * 10.0);
    float checker = mod(p.x + p.y, 2.0);
    vec3 color = mix(vec3(0.1, 0.1, 0.1), vec3(0.9, 0.9, 0.9), checker);
    outColor = vec4(color, 1.0);
}
