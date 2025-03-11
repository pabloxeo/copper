# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.22.1)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(webgpu-backend-wgpu-populate NONE)



include(ExternalProject)
ExternalProject_Add(webgpu-backend-wgpu-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_COMMAND" "cd" "/home/pabloxeo/Documents/copper/run/_deps/webgpu-backend-wgpu-src" "&&" "git" "init" "&&" "git" "fetch" "--depth=1" "https://github.com/eliemichel/WebGPU-distribution" "54a60379a9d792848a2311856375ceef16db150e" "&&" "git" "reset" "--hard" "FETCH_HEAD"
                    SOURCE_DIR          "/home/pabloxeo/Documents/copper/run/_deps/webgpu-backend-wgpu-src"
                    BINARY_DIR          "/home/pabloxeo/Documents/copper/run/_deps/webgpu-backend-wgpu-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)


