cmake_minimum_required(VERSION 3.20)

project(Copper 
    VERSION 0.0.2 
    LANGUAGES CXX C
    DESCRIPTION "3D SDF-based modeling application"
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ===== External Dependencies =====

# Dawn (WebGPU)
set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory(lib/dawn EXCLUDE_FROM_ALL)

# GLM (Math library)
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

# ImGuiFileDialog
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    imguifiledialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG v0.6.6
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imguifiledialog)

# ===== Source Files =====

# Collect source files (excluding main.cpp to avoid duplication)
file(GLOB_RECURSE COPPER_SOURCES
    src/*.cpp
    src/*.h
)

# Remove main.cpp from the list since we add it explicitly
list(REMOVE_ITEM COPPER_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)

# ImGui source files
set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/misc/cpp/imgui_stdlib.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_wgpu.cpp
)

# ===== Executable =====
add_executable(copper 
    src/main.cpp 
    ${COPPER_SOURCES}
    ${IMGUI_SOURCES}
)

# ===== Include Directories =====
target_include_directories(copper PRIVATE
    src
    lib/imgui
    lib/imgui/misc/cpp
    lib/imgui/backends
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

# Tell ImGuiFileDialog where to find ImGui headers
target_include_directories(ImGuiFileDialog PRIVATE
    lib/imgui
    lib/imgui/misc/cpp
    lib/imgui/backends
)

# ===== Compiler Definitions =====
target_compile_definitions(copper PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src"
    IMGUI_IMPL_WEBGPU_BACKEND_DAWN
)


# ===== Link Libraries =====
target_link_libraries(copper PRIVATE
    dawn::webgpu_dawn
    glm::glm
    glfw
    webgpu_glfw
    ImGuiFileDialog
)

