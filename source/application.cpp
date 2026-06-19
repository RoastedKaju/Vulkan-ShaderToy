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

        ++frameCounter;
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