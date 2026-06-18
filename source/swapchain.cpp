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

void Swapchain::destroySwapchain()
{
    for (auto i = 0; i < swapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(pContext->getLogicalDevice(), swapchainImageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(pContext->getLogicalDevice(), swapchain, nullptr);
    vkDestroySurfaceKHR(pContext->getInstance(), surface, nullptr);

    std::cout << "Destroyed vulkan swapchain.\n";
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
    const auto presentMode = pContext->getConfig().presentMode;

    // clamp min image count
    uint32_t minImageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0)
    {
        minImageCount = std::min(minImageCount, surfaceCaps.maxImageCount);
    }

    const VkFormat imageFormat{VK_FORMAT_B8G8R8A8_SRGB};
    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = minImageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = swapchainExtent.width, .height = swapchainExtent.height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode};

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

    format = imageFormat;

    std::cout << "Swapchain created.\n";
}
