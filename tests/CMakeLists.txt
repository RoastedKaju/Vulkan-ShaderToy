find_package(GTest CONFIG REQUIRED)

add_executable(ShaderToyTests tests.cpp)
target_compile_features(ShaderToyTests PRIVATE cxx_std_20)

target_link_libraries(ShaderToyTests PRIVATE ShaderToyLib GTest::gtest GTest::gtest_main)

target_include_directories(ShaderToyTests PRIVATE ${CMAKE_SOURCE_DIR}/source)

include(GoogleTest)
gtest_discover_tests(ShaderToyTests)