cmake_minimum_required(VERSION 3.24)

project(ImGUI)

add_library(ImGUI STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp
    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_sdlrenderer3.cpp
    backends/imgui_impl_vulkan.cpp
)

target_include_directories(ImGUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

target_link_libraries(ImGUI PUBLIC SDL3::SDL3)

target_compile_definitions(ImGUI PUBLIC IMGUI_IMPL_VULKAN_USE_VOLK)
