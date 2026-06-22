#include <iostream>
#include <stdexcept>

#include "application.hpp"

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