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
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t deviceIndex{0};
};

class VulkanContext
{
public:
    VulkanContext() = default;
    VulkanContext(const Config &config);
    ~VulkanContext();

    SDL_Window *createWindow(const char *title, int width, int height);

    // Getters
    inline SDL_Window *getWindow() const { return pWindow; }
    inline VkInstance getInstance() const { return instance; }
    inline VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    inline VkDevice getLogicalDevice() const { return device; }
    inline VkQueue getGraphicsQueue() const { return queue; }
    inline const Config &getConfig() const { return config; }

private:
    void createInstance();
    void createDevice();
    void initializeAllocator();

private:
    SDL_Window *pWindow{nullptr};

    Config config{};
    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
};