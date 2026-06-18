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

    // Getters
    inline VkSurfaceKHR getSurface() const { return surface; }
    inline VkSurfaceCapabilitiesKHR &getSurfaceCaps() { return surfaceCaps; }
    inline VkSwapchainKHR &getSwapchain() { return swapchain; }
    inline std::vector<VkImage> &getSwapchainImages() { return swapchainImages; }
    inline std::vector<VkImageView> &getSwapchainImageViews() { return swapchainImageViews; }
    inline VkFormat getFormat() const { return format; };
    inline bool isSwapchainDirty() const { return swapchainDirty; }
    inline const glm::ivec2 &getWindowSize() const { return windowSize; }
    inline VkExtent2D getSwapchainExtent() const { return swapchainExtent; }

    // Setter
    inline void markSwapchainDirty() { swapchainDirty = true; }
    inline void markSwapchainDirty(bool state) { swapchainDirty = state; }
    inline void setWindowSize(const glm::ivec2 &size) { windowSize = size; }
    inline void setSwapchainExtent(VkExtent2D extent) { swapchainExtent = extent; }

protected:
    void createSurface();
    void createSwapchain();

private:
    VulkanContext *pContext;
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VkExtent2D swapchainExtent{};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkFormat format;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    glm::ivec2 windowSize{};
    bool swapchainDirty{false};
};