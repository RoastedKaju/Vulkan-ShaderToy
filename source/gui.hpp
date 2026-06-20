#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include "vulkan_context.hpp"
#include "swapchain.hpp"

class GUI
{
public:
    void init(VulkanContext &context, Swapchain &swapchain);
    void recordCommands(VkCommandBuffer cmd, Swapchain &swapchain, uint32_t imageIndex);
    void shutdown(VulkanContext &context);

    inline bool IsInitialized() const { return imguiInitialized; }

private:
    void beginFrame();
    void drawUI();
    void endFrame(VkCommandBuffer cmd, Swapchain &swapchain, uint32_t imageIndex);

private:
    VkDescriptorPool imguiDescriptorPool{VK_NULL_HANDLE};
    bool imguiInitialized{false};
};