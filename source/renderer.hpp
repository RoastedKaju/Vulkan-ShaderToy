#pragma once

#include <array>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

#include "vulkan_context.hpp"
#include "swapchain.hpp"

constexpr uint32_t maxFramesInFlight{2};

class Renderer
{
public:
    Renderer(VulkanContext &ctx);
    ~Renderer();

private:
    void createSyncObjects();

private:
    // Renderer uses context
    VulkanContext &context;

    // Renderer own swapchain
    Swapchain swapchain;

    // Frame data
    uint32_t imageIndex{0};
    uint32_t frameIndex{0};
    std::array<VkFence, maxFramesInFlight> fences;
    std::array<VkSemaphore, maxFramesInFlight> imageAcquiredSemaphores;
    std::vector<VkSemaphore> renderComponentSemaphores;
};