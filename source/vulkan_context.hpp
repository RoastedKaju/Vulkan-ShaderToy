#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

struct DebugConfig
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
    VulkanContext(int argc, char *argv[], const DebugConfig &config);
    ~VulkanContext();

protected:
    void createInstance();
    void createDevice();

private:
    int argc;
    char *argv;

    DebugConfig config{};
    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
};