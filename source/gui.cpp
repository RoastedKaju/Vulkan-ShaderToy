#include "gui.hpp"

#include <iostream>
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <volk.h>

#include "utils.hpp"

void GUI::init(VulkanContext &context, Swapchain &swapchain)
{
    VkDevice device = context.getLogicalDevice();
    SDL_Window *window = context.getWindow();

    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * std::size(poolSizes);
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    utils::check(vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiDescriptorPool));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsClassic();
    ImGui_ImplSDL3_InitForVulkan(window);

    // clamp min image count
    uint32_t minImageCount = swapchain.getSurfaceCaps().minImageCount + 1;
    if (swapchain.getSurfaceCaps().maxImageCount > 0)
    {
        minImageCount = std::min(minImageCount, swapchain.getSurfaceCaps().maxImageCount);
    }

    VkFormat colorFormat = swapchain.getFormat();

    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &colorFormat;

    ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
    pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.PipelineRenderingCreateInfo = pipelineRenderingInfo;

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = context.getInstance();
    initInfo.PhysicalDevice = context.getPhysicalDevice();
    initInfo.Device = context.getLogicalDevice();
    initInfo.QueueFamily = context.getGraphicsFamily();
    initInfo.Queue = context.getGraphicsQueue();
    initInfo.DescriptorPool = imguiDescriptorPool;
    initInfo.MinImageCount = minImageCount;
    initInfo.ImageCount = static_cast<uint32_t>(swapchain.getSwapchainImages().size());
    initInfo.PipelineInfoMain = pipelineInfo;
    initInfo.UseDynamicRendering = true;
    initInfo.ApiVersion = VK_API_VERSION_1_3;

    ImGui_ImplVulkan_Init(&initInfo);

    imguiInitialized = true;

    std::cout << "ImGUI context created.\n";
}

void GUI::recordCommands(VkCommandBuffer cmd, Swapchain &swapchain, uint32_t imageIndex)
{
    beginFrame();
    drawUI();
    endFrame(cmd, swapchain, imageIndex);
}

void GUI::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void GUI::drawUI()
{
    ImGui::Begin("Window");
    ImGui::Text("Hello, world!");
    ImGui::End();
}

void GUI::endFrame(VkCommandBuffer cmd, Swapchain &swapchain, uint32_t imageIndex)
{
    ImGui::Render();

    VkRenderingAttachmentInfo attachementInfo{};
    attachementInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    attachementInfo.imageView = swapchain.getSwapchainImageViews()[imageIndex];
    attachementInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachementInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachementInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachementInfo.clearValue = {.color{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, swapchain.getSwapchainExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &attachementInfo;

    vkCmdBeginRendering(cmd, &renderingInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);
}

void GUI::shutdown(VulkanContext &context)
{
    if (!imguiInitialized)
    {
        std::cout << "Not initialized\n";
        return;
    }

    std::cout << "Destroying renderer\n";

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(context.getLogicalDevice(), imguiDescriptorPool, nullptr);
    imguiDescriptorPool = VK_NULL_HANDLE;
    imguiInitialized = false;
}
