#include "application.hpp"

Application::Application()
{
}

Application::~Application()
{
}

void Application::init(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to Init Video: " << SDL_GetError() << "\n";
        exit(EXIT_FAILURE);
    }

    DebugConfig config{.enableValidation = true, .validationLayers{"VK_LAYER_KHRONOS_validation"}};
    pVulkanContext = std::make_unique<VulkanContext>(argc, argv, config);

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

        SDL_SetRenderDrawColor(pRenderer, 0, 128, 100, 255);
        SDL_RenderClear(pRenderer);
        SDL_RenderPresent(pRenderer);
    }
}

void Application::shutdown()
{
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
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