cmake_minimum_required(VERSION 3.13)

project(Copper VERSION 0.0.2 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# Add package Dawn
set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory("dawn" EXCLUDE_FROM_ALL)

add_subdirectory(imgui)


# Create the executable
add_executable(copper 
        src/main.cpp
        src/utils/wgpu-util.h
        src/ui/window.cpp
        src/ui/window.h
        src/core/core.cpp
        src/core/core.h
        src/core/renderer.cpp
        src/core/renderer.h
        src/ui/controls.cpp
        src/ui/controls.h
)

# Declare dependency on the dawn::webgpu_dawn library
target_link_libraries(copper PRIVATE dawn::webgpu_dawn glfw webgpu_glfw imgui)

target_compile_definitions(copper PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src")

# Compiler options
target_compile_options(copper PRIVATE -Wall -Wextra -pedantic)

# Include directories
target_include_directories(copper PRIVATE ${CMAKE_SOURCE_DIR}/src)