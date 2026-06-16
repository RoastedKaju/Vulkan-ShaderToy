cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# Check Vulkan
if(NOT IS_DIRECTORY $ENV{VULKAN_SDK})
    message(WARNING "Vulkan SDK not found!")
else()
    message(STATUS "Found Vulkan in $ENV{VULKAN_SDK}")
endif()

project(ShaderToy)

file(GLOB_RECURSE SOURCE_FILES "source/*.cpp")
file(GLOB_RECURSE HEADER_FILES "source/*.hpp")

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES} ${HEADER_FILES})

# CPP standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Shader C
# Adding system in the Vulkan include directory will not show us warnings originating from there
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE $ENV{VULKAN_SDK}/Include)
target_link_libraries(${PROJECT_NAME} PRIVATE $ENV{VULKAN_SDK}/Lib/shaderc_combined.lib)

# SDL3
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Volk
find_package(volk CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE volk::volk volk::volk_headers)

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# VMA
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)

# Precompile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES)

# Compile options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()