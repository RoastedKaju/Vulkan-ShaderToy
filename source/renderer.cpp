#include "renderer.hpp"

#include <volk.h>

#include "utils.hpp"

Renderer::Renderer(VulkanContext &ctx) : context{ctx}
{
    // Use the context to create swapchain and surface
    swapchain.init(&context);
    createSyncObjects();
}

Renderer::~Renderer()
{
}

void Renderer::createSyncObjects()
{
    auto device = context.getLogicalDevice();
    size_t swapchainImageCount = swapchain.getSwapchainImages().size();

    VkSemaphoreCreateInfo semaphoreCreateInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    for (auto i = 0; i < maxFramesInFlight; ++i)
    {
        utils::check(vkCreateFence(device, &fenceCreateInfo, nullptr, &fences[i]));
        utils::check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAcquiredSemaphores[i]));
    }

    renderComponentSemaphores.resize(swapchainImageCount);

    for (auto &semaphore : renderComponentSemaphores)
    {
        utils::check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
    }
    std::cout << "Sync objects created.\n";
}
