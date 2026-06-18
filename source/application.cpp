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

    // Load fullscreen shaders
    pRenderer->createShaders();

    // Create graphics pipeline
    pRenderer->createPipeline();

    lastTime = SDL_GetTicks();
    isRunning = true;
}

void Application::run()
{
    SDL_Event event;
    while (isRunning)
    {
        float elapsedTime{(SDL_GetTicks() - lastTime) / 1000.0f};
        lastTime = SDL_GetTicks();
        (void)elapsedTime;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                isRunning = false;
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                pRenderer->markSwapchainDirty();
            }
        }

        pRenderer->drawFrame();
    }
}

void Application::shutdown()
{
    pRenderer.reset();
    pVulkanContext.reset();
    
    std::cout << "Application tear down complete.\n";
}

int main(int argc, char *argv)
{
    try
    {
        uint32_t deviceIndex{0};
        if (argc > 1)
        {
            deviceIndex = argv[1];
        }

        Application application{};
        application.init(deviceIndex);
        application.run();
        application.shutdown();
    }
    catch (const std::exception &exception)
    {
        std::cerr << "[EXCEPTION] " << exception.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}