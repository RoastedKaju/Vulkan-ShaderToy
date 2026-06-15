#pragma once

#include <iostream>
#include <memory>
#include <SDL3/SDL.h>

#include "vulkan_context.hpp"

class Application
{
public:
    Application();
    ~Application();

    void init(int argc, char *argv[]);
    void run();
    void shutdown();

private:
    std::unique_ptr<VulkanContext> pVulkanContext;

    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    bool isRunning = false;
};