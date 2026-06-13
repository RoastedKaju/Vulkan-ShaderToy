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