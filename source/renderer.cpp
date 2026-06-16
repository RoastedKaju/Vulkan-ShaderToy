#include "renderer.hpp"

#include <volk.h>

#include "utils.hpp"

Renderer::Renderer(VulkanContext &ctx) : context{ctx}
{
    // Use the context to create swapchain and surface
    swapchain.init(&context);
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

void Renderer::createShaders()
{
    auto vertexSpirv{compileShader("../../assets/shaders/fullscreen.vert", shaderc_vertex_shader)};
    auto fragmentSpirv{compileShader("../../assets/shaders/fullscreen.frag", shaderc_fragment_shader)};

    vertexShaderModule = createShaderModule(vertexSpirv);
    fragmentShaderModule = createShaderModule(fragmentSpirv);

    std::cout << "Shader modules created.\n";\
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
