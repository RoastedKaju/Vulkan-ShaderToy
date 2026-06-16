#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <stdexcept>
#include <filesystem>
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

    inline std::string readTextFile(const std::filesystem::path &path)
    {
        if (!std::filesystem::exists(path))
        {
            const std::string error = "File does not exist: " + path.string();
            throw std::runtime_error(error);
        }

        auto &&stream = std::ifstream(path, std::ios::binary);

        stream.seekg(0, std::ios::end);
        size_t fileLength = stream.tellg();
        stream.seekg(0, std::ios::beg);

        auto &&result = std::string(fileLength, '\0');
        stream.read(result.data(), fileLength);

        return result;
    }

    // Binary file read utility
    // std::vector<std::byte> readBinaryFile(...);
}