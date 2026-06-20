#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <SDL3/SDL.h>

#include "vulkan_context.hpp"
#include "renderer.hpp"
#include "gui.hpp"

class Application
{
public:
    Application();
    ~Application();

    void init(uint32_t deviceIndexArg = 0);
    void run();
    void shutdown();

private:
    std::unique_ptr<VulkanContext> pVulkanContext;
    std::unique_ptr<Renderer> pRenderer;
    std::unique_ptr<GUI> pGUI;

    SDL_Window *pWindow;
    bool isRunning = false;

    uint64_t lastCounter{0};
    float time{0.0f};
    float deltaTime{0.0f};
    uint32_t frameCounter{0};
};