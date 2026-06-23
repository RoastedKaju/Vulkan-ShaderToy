#include <gtest/gtest.h>

#include <memory>

#include "application.hpp"
#include "vulkan_context.hpp"
#include "renderer.hpp"
#include "swapchain.hpp"

TEST(Application, Application_Creation_ReturnsValidInstance)
{
    std::unique_ptr<Application> application = std::make_unique<Application>();

    EXPECT_NE(application.get(), nullptr);
}

TEST(Context, VulkanContext_Creation_ReturnsValidInstance)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);

    EXPECT_NE(ctx.get(), nullptr);
}

TEST(Context, VulkanContext_CreateWindow_ReturnsValidWindow)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);
    ctx->createWindow("Test-Window", 1, 1);

    EXPECT_NE(ctx->getWindow(), nullptr);
}

TEST(Context, VulkanContext_Instance_IsValidHandle)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);

    EXPECT_NE(ctx->getInstance(), VK_NULL_HANDLE);
}

TEST(Context, VulkanContext_Physical_Device_IsValidHandle)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);

    EXPECT_NE(ctx->getPhysicalDevice(), VK_NULL_HANDLE);
}

TEST(Context, VulkanContext_Queue_IsValidHandle)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);

    EXPECT_NE(ctx->getGraphicsQueue(), VK_NULL_HANDLE);
}

TEST(Context, VulkanContext_Device_IsValidHandle)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);

    EXPECT_NE(ctx->getLogicalDevice(), VK_NULL_HANDLE);
}

TEST(Context, VulkanContext_Allocator_IsValidHandle)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);

    EXPECT_NE(ctx->getAllocator(), VK_NULL_HANDLE);
}

TEST(Renderer, Renderer_ImageIndex_IsZero)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);
    ctx->createWindow("Test-Window", 1, 1);
    std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(*ctx);

    EXPECT_EQ(renderer->getImageIndex(), 0);
}

TEST(Swapchain, Renderer_Swapchain_IsValid)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);
    ctx->createWindow("Test-Window", 1, 1);
    std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(*ctx);

    EXPECT_NE(renderer->getSwapchain().getSwapchain(), VK_NULL_HANDLE);
}


TEST(Swapchain, Swapchain_Format_B8G8R8A8_SRGB)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = 0};
    std::unique_ptr<VulkanContext> ctx = std::make_unique<VulkanContext>(config);
    ctx->createWindow("Test-Window", 1, 1);
    std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(*ctx);

    EXPECT_EQ(renderer->getSwapchain().getFormat(), VK_FORMAT_B8G8R8A8_SRGB);
}