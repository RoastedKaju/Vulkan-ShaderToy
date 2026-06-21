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
# target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE $ENV{VULKAN_SDK}/Include)
# target_link_libraries(${PROJECT_NAME} PRIVATE $ENV{VULKAN_SDK}/Lib/shaderc_combined.lib)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "external/shaderc/include")
target_link_directories(${PROJECT_NAME} PRIVATE "external/shaderc/lib")
target_link_libraries(${PROJECT_NAME} PRIVATE shaderc_shared)

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

# ImGUI
add_subdirectory(external/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE ImGUI)

# Precompile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES)

# Compile options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install
# cmake --install build --prefix ./dist
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES $<TARGET_FILE:SDL3::SDL3> DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/external/shaderc/bin/shaderc_shared.dll DESTINATION bin)
install(DIRECTORY assets/ DESTINATION assets)

# Asset Pathing
option(USE_INSTALL_PATH "Use install prefix for assets" OFF)

if(USE_INSTALL_PATH)
    set(ASSETS_DIR "${CMAKE_INSTALL_PREFIX}/assets")
else()
    set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
endif()

configure_file(
    ${CMAKE_SOURCE_DIR}/config.hpp.in
    ${CMAKE_BINARY_DIR}/config.hpp
    @ONLY
)

configure_file(
    ${CMAKE_SOURCE_DIR}/config.hpp.in
    ${CMAKE_BINARY_DIR}/config.hpp
    @ONLY
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR})