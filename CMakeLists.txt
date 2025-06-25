cmake_minimum_required(VERSION 3.13)

project(Copper VERSION 0.0.2 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# Add package Dawn
set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory("dawn" EXCLUDE_FROM_ALL)


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
        src/core/coder.cpp
        src/core/coder.h
)

# Add ImGui source files
target_sources(copper PRIVATE
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_wgpu.cpp
)

# Add ImGui include directories
target_include_directories(copper PRIVATE
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/misc/cpp
)

# Declare dependency on the dawn::webgpu_dawn library
target_link_libraries(copper PRIVATE dawn::webgpu_dawn glfw webgpu_glfw)

target_compile_definitions(copper PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src"
                                          IMGUI_IMPL_WEBGPU_BACKEND_DAWN
                                          )

# Compiler options
target_compile_options(copper PRIVATE -Wall -Wextra -pedantic)

# Include directories
target_include_directories(copper PRIVATE ${CMAKE_SOURCE_DIR}/src)