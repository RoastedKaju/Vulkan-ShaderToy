#include "application.hpp"

#include <SDL3/SDL.h>

Application::Application()
{
}

Application::~Application()
{
}

void Application::init(int argc, char *argv[])
{
    Config config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}};
    pVulkanContext = std::make_unique<VulkanContext>(argc, argv, config);

    pVulkanContext->createWindow("Shader-Toy", 1280u, 720u);

    isRunning = false;
}

void Application::run()
{
    SDL_Event event;
    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                isRunning = false;
            }
        }
    }
}

void Application::shutdown()
{
    pVulkanContext.reset();
}

int main(int argc, char *argv)
{
    (void)argc;
    (void)argv;

    try
    {
        Application application{};
        application.init(argc, &argv);
        application.run();
        application.shutdown();
    }
    catch (const std::exception &exception)
    {
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}