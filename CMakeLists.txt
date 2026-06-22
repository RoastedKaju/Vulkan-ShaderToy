cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# Check Vulkan
if(NOT IS_DIRECTORY $ENV{VULKAN_SDK})
    message(WARNING "Vulkan SDK not found!")
else()
    message(STATUS "Found Vulkan in $ENV{VULKAN_SDK}")
endif()

project(ShaderToy)

# Enable testing
include(CTest)

set(SHADER_TOY_LIB ShaderToyLib)

set(SOURCE_FILES
    "source/application.cpp"
    "source/gui.cpp"
    "source/includer.cpp"
    "source/renderer.cpp"
    "source/swapchain.cpp"
    "source/vulkan_context.cpp"
)
set(HEADER_FILES
    "source/application.hpp"
    "source/gui.hpp"
    "source/includer.hpp"
    "source/renderer.hpp"
    "source/swapchain.hpp"
    "source/vulkan_context.hpp"
    "source/math.hpp"
    "source/utils.hpp"
)

add_library(${SHADER_TOY_LIB} SHARED)
set_target_properties(${SHADER_TOY_LIB} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_sources(${SHADER_TOY_LIB} PRIVATE ${SOURCE_FILES} ${HEADER_FILES})

# CPP standard
target_compile_features(${SHADER_TOY_LIB} PRIVATE cxx_std_20)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${SHADER_TOY_LIB} PRIVATE Vulkan::Vulkan)

# Shader C
# Adding system in the Vulkan include directory will not show us warnings originating from there
# target_include_directories(${SHADER_TOY_LIB} SYSTEM PRIVATE $ENV{VULKAN_SDK}/Include)
# target_link_libraries(${SHADER_TOY_LIB} PRIVATE $ENV{VULKAN_SDK}/Lib/shaderc_combined.lib)
target_include_directories(${SHADER_TOY_LIB} SYSTEM PUBLIC "external/shaderc/include")
target_link_directories(${SHADER_TOY_LIB} PUBLIC "external/shaderc/lib")
target_link_libraries(${SHADER_TOY_LIB} PUBLIC shaderc_shared)

# SDL3
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(${SHADER_TOY_LIB} PUBLIC SDL3::SDL3)

# Volk
find_package(volk CONFIG REQUIRED)
target_link_libraries(${SHADER_TOY_LIB} PRIVATE volk::volk volk::volk_headers)

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${SHADER_TOY_LIB} PRIVATE glm::glm)

# VMA
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(${SHADER_TOY_LIB} PRIVATE GPUOpen::VulkanMemoryAllocator)

# ImGUI
add_subdirectory(external/imgui)
target_link_libraries(${SHADER_TOY_LIB} PUBLIC ImGUI)

# Precompile definitions
target_compile_definitions(${SHADER_TOY_LIB} PUBLIC VK_NO_PROTOTYPES)

# Compile options
if(MSVC)
    target_compile_options(${SHADER_TOY_LIB} PRIVATE /W4)
else()
    target_compile_options(${SHADER_TOY_LIB} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Add executable
add_executable(${PROJECT_NAME} "source/main.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE ${SHADER_TOY_LIB})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/source)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Testing project
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()