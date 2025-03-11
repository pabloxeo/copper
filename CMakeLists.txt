cmake_minimum_required(VERSION 3.0...3.25)
project(
        Copper
        VERSION 0.1.0
        LANGUAGES CXX C
)

# Add GLFW as a subdirectory for local builds
if (NOT EMSCRIPTEN)
    add_subdirectory(glfw)
else()
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

# Add WebGPU subdirectories
add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)

# Find OpenGL
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    message(STATUS "OpenGL found at ${OPENGL_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenGL not found")
endif()

# Add the executable for your project
add_executable(copper
        src/main.cpp
        src/core/App.cpp
        src/core/App.h
        implementations.cpp
        src/ui/Window.cpp src/ui/Window.h src/renderer/Renderer.cpp src/renderer/Renderer.h)

# Link OpenGL to your executable
target_include_directories(copper PRIVATE ${OPENGL_INCLUDE_DIRS})

# Link OpenGL to the executable along with other libraries
target_link_libraries(copper PRIVATE glfw webgpu glfw3webgpu OpenGL::GL)

target_compile_definitions(copper PRIVATE
        RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src"
        )

# Copy WebGPU binaries if needed
target_copy_webgpu_binaries(copper)

# Set the C++ standard
set_target_properties(copper PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_WARNING_AS_ERROR ON
        )

# Set compile options for MSVC and others
if (MSVC)
    target_compile_options(copper PRIVATE /W4)
else()
    target_compile_options(copper PRIVATE -Wall -Wextra -pedantic)
endif()

# Set properties for XCode if using it
if (XCODE)
    set_target_properties(copper PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
            )
endif()

# Adjust for Emscripten if building for web
if (EMSCRIPTEN)
    set_target_properties(copper PROPERTIES SUFFIX ".html")
    target_link_options(copper PRIVATE -sASYNCIFY)
endif()
