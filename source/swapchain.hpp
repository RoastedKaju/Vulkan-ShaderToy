#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "math.hpp"

class VulkanContext;

class Swapchain
{
public:
    Swapchain() = default;
    ~Swapchain();

    void init(VulkanContext *context);

protected:
    void createSurface();
    void createSwapchain();

private:
    VulkanContext *pContext;
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VkExtent2D swapchainExtent{};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    glm::ivec2 windowSize{};
    bool swapchianDirty{false};
};