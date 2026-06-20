#pragma once

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include "vulkan_context.hpp"
#include "renderer.hpp"

class GUI
{
public:
    void init(VulkanContext &context, Swapchain &swapchain);
    void recordCommands(VkCommandBuffer cmd, Renderer &renderer);
    void shutdown(VulkanContext &context);

    inline bool IsInitialized() const { return imguiInitialized; }
    inline bool &getReloadRequestStatus() { return reloadRequested; }

    inline static std::string outputLog{"Successfully compiled!"};

private:
    void beginFrame();
    void drawUI(Renderer &renderer);
    void endFrame(VkCommandBuffer cmd, Swapchain &swapchain, uint32_t imageIndex);

private:
    VkDescriptorPool imguiDescriptorPool{VK_NULL_HANDLE};
    bool imguiInitialized{false};
    bool reloadRequested{false};
};