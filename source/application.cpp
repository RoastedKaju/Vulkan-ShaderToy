#include "application.hpp"

Application::Application()
{
}

Application::~Application()
{
}

void Application::init(uint32_t deviceIndexArg)
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}, .deviceIndex = deviceIndexArg};
    pVulkanContext = std::make_unique<VulkanContext>(config);

    // Create window
    pWindow = pVulkanContext->createWindow("Shader-Toy", 1280u, 720u);

    // Create renderer
    pRenderer = std::make_unique<Renderer>(*pVulkanContext);

    // Create GUI
    pGUI = std::make_unique<GUI>();

    // Load fullscreen shaders
    pRenderer->createShaders();

    // Create graphics pipeline
    pRenderer->createPipeline();

    pGUI->init(*pVulkanContext, pRenderer->getSwapchain());

    // Hook up callbacks
    pRenderer->onRecordCommands = [this](VkCommandBuffer cmd, Renderer &renderer)
    {
        pGUI->recordCommands(cmd, renderer);
    };

    pRenderer->onDestroyRenderer = [this](VulkanContext &ctx)
    {
        pGUI->shutdown(ctx);
    };

    pRenderer->onShaderReload = [this](const std::string &message)
    {
        GUI::outputLog = message;
    };

    lastCounter = SDL_GetPerformanceCounter();
    isRunning = true;
}

void Application::run()
{
    SDL_Event event;
    while (isRunning)
    {
        const Uint64 currentCounter = SDL_GetPerformanceCounter();
        deltaTime = static_cast<float>((currentCounter - lastCounter) / static_cast<double>(SDL_GetPerformanceFrequency()));

        lastCounter = currentCounter;
        time += deltaTime;

        while (SDL_PollEvent(&event))
        {
            if (pGUI->IsInitialized())
            {
                ImGui_ImplSDL3_ProcessEvent(&event);
            }

            if (event.type == SDL_EVENT_QUIT)
            {
                isRunning = false;
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                pRenderer->markSwapchainDirty();
            }
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (!event.key.repeat && event.key.scancode == SDL_SCANCODE_R && event.key.mod & SDL_KMOD_CTRL)
                {
                    std::cout << "Reloading shaders.\n";
                    pRenderer->reloadShaders();
                }
            }
        }

        pRenderer->drawFrame(time, deltaTime, frameCounter);

        // Reload shaders if requested by UI
        if (pGUI->getReloadRequestStatus() == true)
        {
            pRenderer->reloadShaders();
            pGUI->getReloadRequestStatus() = false;
        }

        ++frameCounter;
    }
}

void Application::shutdown()
{
    pRenderer.reset();
    pVulkanContext.reset();

    std::cout << "Application tear down complete.\n";
}