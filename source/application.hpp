#pragma once

#include <iostream>
#include <memory>

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
    bool isRunning = false;
};