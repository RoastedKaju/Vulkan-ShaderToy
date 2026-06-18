#include "vulkan_context.hpp"

#include <cassert>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <SDL3/SDL_vulkan.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "utils.hpp"

// Debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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

VulkanContext::VulkanContext(const Config &config) : config{config}
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to Init Video: " << SDL_GetError() << "\n";
        exit(EXIT_FAILURE);
    }

    utils::check(SDL_Vulkan_LoadLibrary(NULL));
    if (volkInitialize() != VK_SUCCESS)
    {
        std::cerr << "Failed to initialize Volk\n";
        exit(EXIT_FAILURE);
    }

    createInstance();
    createDevice();
    initializeAllocator();
}

VulkanContext::~VulkanContext()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    vmaDestroyAllocator(allocator);

    if (pWindow)
    {
        SDL_DestroyWindow(pWindow);
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();

    vkDestroyDevice(device, nullptr);
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);

    std::cout << "Destroyed vulkan context.\n";
}

SDL_Window *VulkanContext::createWindow(const char *title, int width, int height)
{
    pWindow = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    assert(pWindow && "Failed to create Window.");
    std::cout << "Window created.\n";

    return pWindow;
}

void VulkanContext::createCommandPool()
{
    VkCommandPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily};

    utils::check(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));
    std::cout << "Command pool created.\n";
}

void VulkanContext::createInstance()
{
    VkApplicationInfo applicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Shader Toy",
        .apiVersion = VK_API_VERSION_1_3};

    uint32_t instanceExtensionCount{0};
    char const *const *sdlExtensions{SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount)};
    std::vector<const char *> instanceExtensions(sdlExtensions, sdlExtensions + instanceExtensionCount);
    if (config.enableValidation)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        ++instanceExtensionCount;
    }

    // Debug util create Info
    // clang-format off
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &debugCallback};
    // clang-format on

    VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = config.enableValidation ? &debugCreateInfo : nullptr,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = config.enableValidation ? static_cast<uint32_t>(config.validationLayers.size()) : 0,
        .ppEnabledLayerNames = config.enableValidation ? config.validationLayers.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data()};

    utils::check(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
    volkLoadInstance(instance);

    // Create debug manager
    if (config.enableValidation)
    {
        utils::check(vkCreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger));
        std::cout << "Validation layers enabled.\n";
    }
}

void VulkanContext::createDevice()
{
    // Physical Device
    uint32_t deviceCount{0};
    utils::check(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    utils::check(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
    uint32_t deviceIndex{0};

    deviceIndex = config.deviceIndex;
    assert(deviceIndex < deviceCount && "Device index is less than device count.");

    VkPhysicalDeviceProperties2 deviceProps{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &deviceProps);
    std::cout << "Selected device: " << deviceProps.properties.deviceName << ".\n";

    // Queue
    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, queueFamilies.data());
    for (size_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamily = static_cast<uint32_t>(i);
            break;
        }
    }
    utils::check(SDL_Vulkan_GetPresentationSupport(instance, devices[deviceIndex], queueFamily));

    // Logical device
    const float queuePriorities{1.0f};
    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriorities};

    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingPartiallyBound = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true};

    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledVk12Features,
        .synchronization2 = true,
        .dynamicRendering = true};

    VkPhysicalDeviceFeatures enabledVk10Features{.samplerAnisotropy = VK_TRUE};

    const std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = (uint32_t)deviceExtensions.size(),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features};

    utils::check(vkCreateDevice(devices[deviceIndex], &deviceCreateInfo, nullptr, &device));
    vkGetDeviceQueue(device, queueFamily, 0, &queue);
    std::cout << "Logical device and Queue created.\n";

    physicalDevice = devices[deviceIndex];
}

void VulkanContext::initializeAllocator()
{
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage};

    VmaAllocatorCreateInfo allocatorCreateInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &vkFunctions,
        .instance = instance};

    utils::check(vmaCreateAllocator(&allocatorCreateInfo, &allocator));
    std::cout << "Allocator created.\n";
}
