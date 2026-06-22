find_package(GTest CONFIG REQUIRED)

add_executable(ShaderToyTests
    application_tests.cpp
    renderer_tests.cpp
    swapchain_tests.cpp
)

target_link_libraries(ShaderToyTests PRIVATE
    ShaderToyLib
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ShaderToyTests)