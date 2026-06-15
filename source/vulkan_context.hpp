#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <SDL3/SDL.h>

#include "swapchain.hpp"

struct Config
{
    bool enableValidation{};
    std::vector<const char *> validationLayers{};
};

// Debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    (void)type;
    (void)pUserData;

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "[Validation] " << pCallbackData->pMessage << "\n";
    }

    return VK_FALSE;
}

class VulkanContext
{
public:
    VulkanContext() = default;
    VulkanContext(int argc, char *argv[], const Config &config);
    ~VulkanContext();

    SDL_Window *createWindow(const char *title, int width, int height);

    // Getters
    inline SDL_Window *getWindow() const { return pWindow; }
    inline VkInstance getInstance() const { return instance; }
    inline VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    inline VkDevice getLogicalDevice() const { return device; }
    inline VkQueue getGraphicsQueue() const { return queue; }

protected:
    void createInstance();
    void createDevice();
    void initializeAllocator();

private:
    int argc;
    char *argv;

    SDL_Window *pWindow{nullptr};

    Config config{};
    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};

    // ....
    Swapchain swapchain;
};