#include "renderer.hpp"

#include <volk.h>

#include "utils.hpp"

Renderer::Renderer(VulkanContext &ctx) : context{ctx}
{
    // Use the context to create swapchain and surface
    swapchain.init(&context);
    // Create buffers that hold shader data per frame
    createShaderDataBuffers();
    // Create semaphores and fences
    createSyncObjects();
    // Create command pool
    context.createCommandPool();
    // Create command buffers
    createCommandBuffers();
    // Create descriptor pool and setup sets
    setupDescriptors();
}

Renderer::~Renderer()
{
}

void Renderer::createShaderDataBuffers()
{
    auto device = context.getLogicalDevice();
    auto allocator = context.getAllocator();

    for (auto i = 0; i < maxFramesInFlight; ++i)
    {
        VkBufferCreateInfo uBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(ShaderData),
            .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};

        VmaAllocationCreateInfo uBufferAllocCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO};

        utils::check(vmaCreateBuffer(allocator, &uBufferCreateInfo, &uBufferAllocCreateInfo, &shaderDataBuffers[i].buffer, &shaderDataBuffers[i].allocation, &shaderDataBuffers[i].allocationInfo));

        VkBufferDeviceAddressInfo uBDAInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = shaderDataBuffers[i].buffer};

        shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(device, &uBDAInfo);
    }

    std::cout << "Shader data buffers created.\n";
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

    renderCompleteSemaphores.resize(swapchainImageCount);

    for (auto &semaphore : renderCompleteSemaphores)
    {
        utils::check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
    }
    std::cout << "Sync objects created.\n";
}

void Renderer::createCommandBuffers()
{
    auto commandPool = context.getCommandPool();
    auto device = context.getLogicalDevice();

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .commandBufferCount = maxFramesInFlight};

    utils::check(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));
    std::cout << "Command buffers created.\n";
}

void Renderer::setupDescriptors()
{
    auto device = context.getLogicalDevice();

    // Descriptor indexing flags
    // Partial bound flag which allows us to have gaps in our texture descriptors
    VkDescriptorBindingFlags descBindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 1,
        .pBindingFlags = &descBindingFlags};

    // Texture array binding
    VkDescriptorSetLayoutBinding textureBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = maxTextures,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};

    // Descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &bindingFlagsInfo,
        .bindingCount = 1,
        .pBindings = &textureBinding};

    utils::check(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

    // Descriptor pool
    VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = maxTextures};

    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize};

    utils::check(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));

    // Descriptor set allocation
    uint32_t variableDescriptorCount = maxTextures;

    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = &variableDescriptorCount};

    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variableCountInfo,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout};

    utils::check(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

    // Update descriptors only if textures exist
    if (!textureDescriptors.empty())
    {
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .descriptorCount = static_cast<uint32_t>(textureDescriptors.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = textureDescriptors.data()};

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    std::cout << "Descriptor sets setup.\n";
}

void Renderer::waitForFrame()
{
    utils::check(vkWaitForFences(context.getLogicalDevice(), 1, &fences[frameIndex], true, UINT64_MAX));
    utils::check(vkResetFences(context.getLogicalDevice(), 1, &fences[frameIndex]));
}

void Renderer::acquireImage()
{
    VkResult result = vkAcquireNextImageKHR(context.getLogicalDevice(), swapchain.getSwapchain(), UINT64_MAX, imageAcquiredSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);
    if (result < VK_SUCCESS)
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            swapchain.markSwapchainDirty();
            return;
        }
        throw std::runtime_error(std::string("Failed to acquire image: " + result));
    }
}

void Renderer::updateShaderData()
{
    shaderData.color = glm::vec3(1.0f, 0.0f, 0.0f);
    memcpy(shaderDataBuffers[frameIndex].allocationInfo.pMappedData, &shaderData, sizeof(ShaderData));
}

void Renderer::recordCommandBuffer(VkCommandBuffer cmd)
{
    utils::check(vkResetCommandBuffer(cmd, 0));
    // Begin command buffer
    VkCommandBufferBeginInfo cmdBufferbeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    utils::check(vkBeginCommandBuffer(cmd, &cmdBufferbeginInfo));

    // Barriers
    // clang-format off
    std::array<VkImageMemoryBarrier2, 1> outputBarriers
    {
        VkImageMemoryBarrier2
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image = swapchain.getSwapchainImages()[imageIndex],
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        }
        // In case you have another rexture like depth attachment, make a barrier here
    };
    // clang-format on

    // Barrier/Transition
    VkDependencyInfo barrierDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = (uint32_t)outputBarriers.size(),
        .pImageMemoryBarriers = outputBarriers.data()};
    vkCmdPipelineBarrier2(cmd, &barrierDependencyInfo);

    VkRenderingAttachmentInfo colorAttachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain.getSwapchainImageViews()[imageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.color{0.0f, 0.0f, 0.0f, 1.0f}}};

    // Ignore depth attachment

    // Rendering Information
    glm::ivec2 windowSize = swapchain.getWindowSize();
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{.extent{.width = (uint32_t)windowSize.x, .height = (uint32_t)windowSize.y}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        .pDepthAttachment = nullptr};

    // Dynamic rendering
    // Begin rendering
    vkCmdBeginRendering(cmd, &renderingInfo);
    // Set viewport and scissor
    VkViewport viewport{.width = (float)windowSize.x, .height = (float)windowSize.y, .minDepth = 0.0f, .maxDepth = 1.0f};
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{.extent{.width = static_cast<uint32_t>(windowSize.x), .height = static_cast<uint32_t>(windowSize.y)}};
    // Bind pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    // Ignore binding vertex, index and push constant

    // Draw
    vkCmdDraw(cmd, 3, 1, 0, 0);

    // End rendering
    vkCmdEndRendering(cmd);

    // Transition image to output format
    VkImageMemoryBarrier2 barrierPresent{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = swapchain.getSwapchainImages()[imageIndex],
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}};
    VkDependencyInfo barrierPresentDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrierPresent};
    vkCmdPipelineBarrier2(cmd, &barrierPresentDependencyInfo);
    // End command buffer
    utils::check(vkEndCommandBuffer(cmd));
}

void Renderer::submitFrame(VkCommandBuffer cmd)
{
    // Submit to graphics queue
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAcquiredSemaphores[frameIndex],
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderCompleteSemaphores[imageIndex]};

    utils::check(vkQueueSubmit(context.getGraphicsQueue(), 1, &submitInfo, fences[frameIndex]));
}

void Renderer::presentFrame()
{
    auto _swapchain = swapchain.getSwapchain();

    // Present
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderCompleteSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &_swapchain,
        .pImageIndices = &imageIndex};

    VkResult result = vkQueuePresentKHR(context.getGraphicsQueue(), &presentInfo);
    if (result < VK_SUCCESS)
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            swapchain.markSwapchainDirty();
            return;
        }
        throw std::runtime_error(std::string("Failed to acquire image: " + result));
    }
}

void Renderer::recreateSwapchain()
{
    auto windowSize = swapchain.getWindowSize();
    auto device = context.getLogicalDevice();
    auto physicalDevice = context.getPhysicalDevice();
    auto oldSwapchain = swapchain.getSwapchain();
    const auto oldImageCount = swapchain.getSwapchainImages().size();

    utils::check(SDL_GetWindowSize(context.getWindow(), &windowSize.x, &windowSize.y));
    swapchain.setWindowSize(windowSize);
    swapchain.markSwapchainDirty(false);

    utils::check(vkDeviceWaitIdle(device));
    utils::check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, swapchain.getSurface(), &swapchain.getSurfaceCaps()));
    swapchain.setSwapchainExtent({(uint32_t)windowSize.x, (uint32_t)windowSize.y});

    // clamp min image count
    uint32_t minImageCount = swapchain.getSurfaceCaps().minImageCount + 1;
    if (swapchain.getSurfaceCaps().maxImageCount > 0)
    {
        minImageCount = std::min(minImageCount, swapchain.getSurfaceCaps().maxImageCount);
    }

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = swapchain.getSurface(),
        .minImageCount = minImageCount,
        .imageFormat = swapchain.getFormat(),
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = swapchain.getSwapchainExtent().width, .height = swapchain.getSwapchainExtent().height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapchain.getSurfaceCaps().currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = context.getConfig().presentMode};

    createInfo.oldSwapchain = oldSwapchain;

    // Create new swapchain
    utils::check(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain.getSwapchain()));

    // Destroy old views
    uint32_t imageCount{(uint32_t)oldImageCount};
    for (auto i = 0; i < (int)imageCount; ++i)
    {
        vkDestroyImageView(device, swapchain.getSwapchainImageViews()[i], nullptr);
    }

    // Get new swapchain images
    utils::check(vkGetSwapchainImagesKHR(device, swapchain.getSwapchain(), &imageCount, nullptr));
    swapchain.getSwapchainImages().resize(imageCount);
    utils::check(vkGetSwapchainImagesKHR(device, swapchain.getSwapchain(), &imageCount, swapchain.getSwapchainImages().data()));
    swapchain.getSwapchainImageViews().resize(imageCount);
    // Create new image views
    for (auto i = 0; i < (int)imageCount; ++i)
    {
        VkImageViewCreateInfo viewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain.getSwapchainImages()[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain.getFormat(),
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}};
        utils::check(vkCreateImageView(device, &viewCreateInfo, nullptr, &swapchain.getSwapchainImageViews()[i]));
    }

    // Destroy old sync objects
    for (auto &semaphore : renderCompleteSemaphores)
    {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
    renderCompleteSemaphores.resize(imageCount);
    VkSemaphoreCreateInfo semaphoreCreateInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto &semaphore : renderCompleteSemaphores)
    {
        utils::check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
    }

    // Destroy old swapchain
    vkDestroySwapchainKHR(device, createInfo.oldSwapchain, nullptr);
    // Normally you would also destroy depth images here

    std::cout << "Swapchain recreated.\n";
}

void Renderer::createPipeline()
{
    VkPipelineLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout};

    utils::check(vkCreatePipelineLayout(context.getLogicalDevice(), &layoutInfo, nullptr, &pipelineLayout));

    // Shader stage
    // clang-format off
    VkPipelineShaderStageCreateInfo shaderStages[]{
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main"
        }
    };
    // clang-format on

    // Vertex Input
    // No bindings, attributes and vertex buffer
    VkPipelineVertexInputStateCreateInfo vertexInputState{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    // Dynamic states
    std::array dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)dynamicStates.size(),
        .pDynamicStates = dynamicStates.data()};
    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1};

    // Rasterization state (no culling for fullscreen passes)
    VkPipelineRasterizationStateCreateInfo rasterizationState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f};

    // Multisampling state
    VkPipelineMultisampleStateCreateInfo multisampleState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};

    // No need for depth state

    // Color blend attachment state
    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT};

    VkPipelineColorBlendStateCreateInfo colorBlendState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment};

    // Rendering and pipeline info
    // No depth format
    VkFormat imageFormat = swapchain.getFormat();
    VkPipelineRenderingCreateInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &imageFormat};

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingInfo,

        .stageCount = 2,
        .pStages = shaderStages,

        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,

        .layout = pipelineLayout};

    // Create graphics pipeline
    utils::check(vkCreateGraphicsPipelines(context.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

    std::cout << "Graphics pipeline created.\n";
}

void Renderer::drawFrame()
{
    waitForFrame();

    acquireImage();

    updateShaderData();

    auto cmd = commandBuffers[frameIndex];

    recordCommandBuffer(cmd);

    submitFrame(cmd);

    presentFrame();

    frameIndex = (frameIndex + 1) % maxFramesInFlight;

    if (swapchain.isSwapchainDirty())
    {
        recreateSwapchain();
    }
}

void Renderer::createShaders()
{
    auto vertexSpirv{compileShader("../../assets/shaders/fullscreen.vert", shaderc_vertex_shader)};
    auto fragmentSpirv{compileShader("../../assets/shaders/fullscreen.frag", shaderc_fragment_shader)};

    vertexShaderModule = createShaderModule(vertexSpirv);
    fragmentShaderModule = createShaderModule(fragmentSpirv);

    std::cout << "Shader modules created.\n";
}

std::vector<uint32_t> Renderer::compileShader(const std::filesystem::path &path, shaderc_shader_kind kind)
{
    const std::string source{utils::readTextFile(path)};

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    auto result{compiler.CompileGlslToSpv(source, kind, path.string().c_str(), options)};

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        throw std::runtime_error(result.GetErrorMessage());
    }

    return {result.cbegin(), result.cend()};
}

VkShaderModule Renderer::createShaderModule(const std::vector<uint32_t> &spirv)
{
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv.size() * sizeof(uint32_t),
        .pCode = spirv.data()};

    VkShaderModule module{VK_NULL_HANDLE};

    utils::check(vkCreateShaderModule(context.getLogicalDevice(), &createInfo, nullptr, &module));

    return module;
}
