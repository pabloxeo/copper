cmake_minimum_required(VERSION 3.0...3.25)
project(
        LearnWebGPU
        VERSION 0.1.0
        LANGUAGES CXX C
)

if (NOT EMSCRIPTEN)
    add_subdirectory(glfw)
else()
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)

add_executable(copper
        main.cpp
        App.cpp
        App.h
        implementations.cpp
)

target_link_libraries(copper PRIVATE glfw webgpu glfw3webgpu)

target_copy_webgpu_binaries(copper)

set_target_properties(copper PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
    target_compile_options(copper PRIVATE /W4)
else()
    target_compile_options(copper PRIVATE -Wall -Wextra -pedantic)
endif()

if (XCODE)
    set_target_properties(copper PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()

if (EMSCRIPTEN)
    set_target_properties(copper PROPERTIES SUFFIX ".html")
    target_link_options(copper PRIVATE -sASYNCIFY)
endif()