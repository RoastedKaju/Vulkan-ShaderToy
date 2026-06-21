# Shader Toy

Shader Toy application built with Vulkan 1.3 dynamic rendering, this project provides a lightweight playground for experimenting with GLSL shaders in real-time.

## Showcase
<p align="center">
  <img src="docs/01.jpg" alt="Plasma Shader" width="45%" />
  <img src="docs/02.jpg" alt="Raymarched Sphere" width="45%" />
</p>

<p align="center">
  <img src="docs/03.jpg" alt="Neon Tunnel" width="45%" />
  <img src="docs/04.jpg" alt="Fractal Showcase" width="45%" />
</p>

## Features
- Vulkan 1.3 backend
- Runtime GLSL to SPIR-V compilation
- UI text editor and reloading button

## Dependencies
- Vulkan 1.3
- SDL3
- ShaderC
- ImGui
- CMake
- C++ 20

## Build Instructions
- Clone the repository using recursive pull
- Run VCPKG bootstrap
- Run `vcpkg install` command to pull the dependencies mentioned in manifest file.
- Configure CMake
- Build