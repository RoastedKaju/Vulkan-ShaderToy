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

    renderComponentSemaphores.resize(swapchainImageCount);

    for (auto &semaphore : renderComponentSemaphores)
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
