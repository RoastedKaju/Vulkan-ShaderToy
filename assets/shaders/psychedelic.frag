#version 460

#include "common.glsl"

void main()
{
    // Access our buffer data safely via the push constant reference
    float time = pc.shaderData.data.time;
    
    // Center the UV coordinates from [0, 1] to [-1, 1] and adjust aspect ratio
    vec2 p = uv * 2.0 - 1.0;
    
    // Create a spinning, evolving coordinate space based on time
    float c = cos(time * 0.5);
    float s = sin(time * 0.5);
    mat2 rotation = mat2(c, -s, s, c);
    vec2 rotatedP = rotation * p;

    // Calculate distance from center for a radial warp
    float d = length(rotatedP);

    // Create moving wave patterns using sine/cosine functions driven by time
    float waveX = sin(rotatedP.x * 4.0 + time) * 0.5;
    float waveY = cos(rotatedP.y * 4.0 + time) * 0.5;
    
    // Generate a dynamic plasma/fractal visual value
    float strength = sin(d * 8.0 - time * 2.0) + waveX + waveY;
    strength = abs(sin(strength * 2.0)); // Sharpen the edges

    // Build a vibrant, shifting color palette
    vec3 colorA = vec3(0.1, 0.4, 0.9); // Deep Cosmic Blue
    vec3 colorB = vec3(0.9, 0.1, 0.5); // Neon Pink/Magenta
    vec3 colorC = vec3(0.0, 0.9, 0.6); // Cyan / Emerald Green

    // Mix the colors dynamically over time and space
    vec3 finalColor = mix(colorA, colorB, strength);
    finalColor += colorC * (sin(time + d * 5.0) * 0.5 + 0.5) * 0.4;

    // Add a soft vignette/glow effect towards the center
    finalColor *= 1.0 - smoothstep(0.5, 1.5, d);

    // Output final RGBA color
    outColor = vec4(finalColor, 1.0);
}