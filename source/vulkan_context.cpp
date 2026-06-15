#include "vulkan_context.hpp"

#include <cassert>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <SDL3/SDL_vulkan.h>

#include "utils.hpp"

VulkanContext::VulkanContext(int argc, char *argv[], const DebugConfig &config) : config{config}
{
    this->argc = argc;
    this->argv = *argv;

    utils::check(SDL_Vulkan_LoadLibrary(NULL));
    if (volkInitialize() != VK_SUCCESS)
    {
        std::cerr << "Failed to initialize Volk\n";
        exit(EXIT_FAILURE);
    }

    createInstance();
    createDevice();
}

VulkanContext::~VulkanContext()
{
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

    if (argc > 1)
    {
        deviceIndex = std::stoi(&argv[1]);
        assert(deviceIndex < deviceCount);
    }

    VkPhysicalDeviceProperties2 deviceProps{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &deviceProps);
    std::cout << "Selected device: " << deviceProps.properties.deviceName << '\n';


    // Queue
    uint32_t queueFamilyCount{0};
}
