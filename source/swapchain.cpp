#include "swapchain.hpp"

#include <volk.h>
#include <SDL3/SDL_vulkan.h>

#include "utils.hpp"
#include "vulkan_context.hpp"

Swapchain::~Swapchain()
{
}

void Swapchain::init(VulkanContext *context)
{
    pContext = context;
    assert(pContext && "Vulkan context is invalid.");

    createSurface();
    createSwapchain();
}

void Swapchain::createSurface()
{
    auto *window = pContext->getWindow();
    auto instance = pContext->getInstance();
    auto physicalDevice = pContext->getPhysicalDevice();

    utils::check(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
    utils::check(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));

    utils::check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));
    swapchainExtent = surfaceCaps.currentExtent;
    if (surfaceCaps.currentExtent.width == 0xFFFFFFFF)
    {
        swapchainExtent = {.width = (uint32_t)windowSize.x, .height = (uint32_t)windowSize.y};
    }
    std::cout << "Surface created.\n";
}

void Swapchain::createSwapchain()
{
    auto device = pContext->getLogicalDevice();

    const VkFormat imageFormat{VK_FORMAT_B8G8R8A8_SRGB};
    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = swapchainExtent.width, .height = swapchainExtent.height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR};

    utils::check(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));

    // Get swapchain images and create views
    uint32_t imageCount{0};
    utils::check(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
    swapchainImages.resize(imageCount);
    utils::check(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));
    swapchainImageViews.resize(imageCount);

    for (auto i = 0; i < (size_t)imageCount; ++i)
    {
        VkImageViewCreateInfo viewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = imageFormat,
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}};

        utils::check(vkCreateImageView(device, &viewCreateInfo, nullptr, &swapchainImageViews[i]));
    }

    std::cout << "Swapchain created.\n";
}
