#pragma once

#include <string>
#include <exception>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace utils
{
    inline void check(VkResult result)
    {
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Vulkan call returned an error (" + std::to_string(result) + ")");
        }
    }

    inline void check(bool result)
    {
        if (!result)
        {
            throw std::runtime_error("Vulkan call returned an error");
        }
    }
}