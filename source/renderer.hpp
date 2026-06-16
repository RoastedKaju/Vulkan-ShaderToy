#pragma once

#include <array>
#include <vector>
#include <memory>
#include <filesystem>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include "vulkan_context.hpp"
#include "swapchain.hpp"

constexpr uint32_t maxFramesInFlight{2};
constexpr uint32_t maxTextures{64};

class Renderer
{
public:
    Renderer(VulkanContext &ctx);
    ~Renderer();

    void createShaders();

private:
    void createSyncObjects();
    void createCommandBuffers();
    void setupDescriptors();

    std::vector<uint32_t> compileShader(const std::filesystem::path &path, shaderc_shader_kind kind);
    VkShaderModule createShaderModule(const std::vector<uint32_t> &spirv);

private:
    // Renderer uses context
    VulkanContext &context;

    // Renderer own swapchain
    Swapchain swapchain;

    // Frame data
    uint32_t imageIndex{0};
    uint32_t frameIndex{0};
    std::array<VkFence, maxFramesInFlight> fences;
    std::array<VkSemaphore, maxFramesInFlight> imageAcquiredSemaphores;
    std::vector<VkSemaphore> renderComponentSemaphores;

    // Command buffers
    std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;

    // Descriptor
    std::vector<VkDescriptorImageInfo> textureDescriptors{};
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};

    // Shader modules
    VkShaderModule vertexShaderModule{VK_NULL_HANDLE};
    VkShaderModule fragmentShaderModule{VK_NULL_HANDLE};

    // Pipeline
};