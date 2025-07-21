cmake_minimum_required(VERSION 3.13)

project(Copper VERSION 0.0.2 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# Add package Dawn
set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory("dawn" EXCLUDE_FROM_ALL)

file(GLOB SRCS
        ${PROJECT_SOURCE_DIR}/src/**/**.cpp
        ${PROJECT_SOURCE_DIR}/src/**/**.h
)

add_executable(copper src/main.cpp ${SRCS})

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
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/misc/cpp
)

# add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

# Declare dependency on the dawn::webgpu_dawn library
target_link_libraries(copper PRIVATE m dawn::webgpu_dawn glm glfw webgpu_glfw)

target_compile_definitions(copper PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src"
                                          IMGUI_IMPL_WEBGPU_BACKEND_DAWN
                                          )

# Compiler options
target_compile_options(copper PRIVATE -Wall -Wextra -pedantic)

# Include directories
target_include_directories(copper PRIVATE ${CMAKE_SOURCE_DIR}/src)