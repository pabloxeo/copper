# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pabloxeo/Documents/copper

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pabloxeo/Documents/copper/run

# Include any dependencies generated for this target.
include CMakeFiles/copper.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/copper.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/copper.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/copper.dir/flags.make

CMakeFiles/copper.dir/src/main.cpp.o: CMakeFiles/copper.dir/flags.make
CMakeFiles/copper.dir/src/main.cpp.o: ../src/main.cpp
CMakeFiles/copper.dir/src/main.cpp.o: CMakeFiles/copper.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pabloxeo/Documents/copper/run/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/copper.dir/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/copper.dir/src/main.cpp.o -MF CMakeFiles/copper.dir/src/main.cpp.o.d -o CMakeFiles/copper.dir/src/main.cpp.o -c /home/pabloxeo/Documents/copper/src/main.cpp

CMakeFiles/copper.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/copper.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pabloxeo/Documents/copper/src/main.cpp > CMakeFiles/copper.dir/src/main.cpp.i

CMakeFiles/copper.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/copper.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pabloxeo/Documents/copper/src/main.cpp -o CMakeFiles/copper.dir/src/main.cpp.s

CMakeFiles/copper.dir/src/core/App.cpp.o: CMakeFiles/copper.dir/flags.make
CMakeFiles/copper.dir/src/core/App.cpp.o: ../src/core/App.cpp
CMakeFiles/copper.dir/src/core/App.cpp.o: CMakeFiles/copper.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pabloxeo/Documents/copper/run/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/copper.dir/src/core/App.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/copper.dir/src/core/App.cpp.o -MF CMakeFiles/copper.dir/src/core/App.cpp.o.d -o CMakeFiles/copper.dir/src/core/App.cpp.o -c /home/pabloxeo/Documents/copper/src/core/App.cpp

CMakeFiles/copper.dir/src/core/App.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/copper.dir/src/core/App.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pabloxeo/Documents/copper/src/core/App.cpp > CMakeFiles/copper.dir/src/core/App.cpp.i

CMakeFiles/copper.dir/src/core/App.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/copper.dir/src/core/App.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pabloxeo/Documents/copper/src/core/App.cpp -o CMakeFiles/copper.dir/src/core/App.cpp.s

CMakeFiles/copper.dir/implementations.cpp.o: CMakeFiles/copper.dir/flags.make
CMakeFiles/copper.dir/implementations.cpp.o: ../implementations.cpp
CMakeFiles/copper.dir/implementations.cpp.o: CMakeFiles/copper.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pabloxeo/Documents/copper/run/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/copper.dir/implementations.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/copper.dir/implementations.cpp.o -MF CMakeFiles/copper.dir/implementations.cpp.o.d -o CMakeFiles/copper.dir/implementations.cpp.o -c /home/pabloxeo/Documents/copper/implementations.cpp

CMakeFiles/copper.dir/implementations.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/copper.dir/implementations.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pabloxeo/Documents/copper/implementations.cpp > CMakeFiles/copper.dir/implementations.cpp.i

CMakeFiles/copper.dir/implementations.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/copper.dir/implementations.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pabloxeo/Documents/copper/implementations.cpp -o CMakeFiles/copper.dir/implementations.cpp.s

CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o: CMakeFiles/copper.dir/flags.make
CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o: ../src/graphics/Renderer.cpp
CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o: CMakeFiles/copper.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pabloxeo/Documents/copper/run/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o -MF CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o.d -o CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o -c /home/pabloxeo/Documents/copper/src/graphics/Renderer.cpp

CMakeFiles/copper.dir/src/graphics/Renderer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/copper.dir/src/graphics/Renderer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pabloxeo/Documents/copper/src/graphics/Renderer.cpp > CMakeFiles/copper.dir/src/graphics/Renderer.cpp.i

CMakeFiles/copper.dir/src/graphics/Renderer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/copper.dir/src/graphics/Renderer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pabloxeo/Documents/copper/src/graphics/Renderer.cpp -o CMakeFiles/copper.dir/src/graphics/Renderer.cpp.s

CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o: CMakeFiles/copper.dir/flags.make
CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o: ../src/graphics/Pipeline.cpp
CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o: CMakeFiles/copper.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pabloxeo/Documents/copper/run/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o -MF CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o.d -o CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o -c /home/pabloxeo/Documents/copper/src/graphics/Pipeline.cpp

CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pabloxeo/Documents/copper/src/graphics/Pipeline.cpp > CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.i

CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pabloxeo/Documents/copper/src/graphics/Pipeline.cpp -o CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.s

CMakeFiles/copper.dir/src/input/InputManager.cpp.o: CMakeFiles/copper.dir/flags.make
CMakeFiles/copper.dir/src/input/InputManager.cpp.o: ../src/input/InputManager.cpp
CMakeFiles/copper.dir/src/input/InputManager.cpp.o: CMakeFiles/copper.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pabloxeo/Documents/copper/run/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/copper.dir/src/input/InputManager.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/copper.dir/src/input/InputManager.cpp.o -MF CMakeFiles/copper.dir/src/input/InputManager.cpp.o.d -o CMakeFiles/copper.dir/src/input/InputManager.cpp.o -c /home/pabloxeo/Documents/copper/src/input/InputManager.cpp

CMakeFiles/copper.dir/src/input/InputManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/copper.dir/src/input/InputManager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pabloxeo/Documents/copper/src/input/InputManager.cpp > CMakeFiles/copper.dir/src/input/InputManager.cpp.i

CMakeFiles/copper.dir/src/input/InputManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/copper.dir/src/input/InputManager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pabloxeo/Documents/copper/src/input/InputManager.cpp -o CMakeFiles/copper.dir/src/input/InputManager.cpp.s

# Object files for target copper
copper_OBJECTS = \
"CMakeFiles/copper.dir/src/main.cpp.o" \
"CMakeFiles/copper.dir/src/core/App.cpp.o" \
"CMakeFiles/copper.dir/implementations.cpp.o" \
"CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o" \
"CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o" \
"CMakeFiles/copper.dir/src/input/InputManager.cpp.o"

# External object files for target copper
copper_EXTERNAL_OBJECTS =

copper: CMakeFiles/copper.dir/src/main.cpp.o
copper: CMakeFiles/copper.dir/src/core/App.cpp.o
copper: CMakeFiles/copper.dir/implementations.cpp.o
copper: CMakeFiles/copper.dir/src/graphics/Renderer.cpp.o
copper: CMakeFiles/copper.dir/src/graphics/Pipeline.cpp.o
copper: CMakeFiles/copper.dir/src/input/InputManager.cpp.o
copper: CMakeFiles/copper.dir/build.make
copper: glfw/src/libglfw3.a
copper: glfw3webgpu/libglfw3webgpu.a
copper: glfw/src/libglfw3.a
copper: /usr/lib/x86_64-linux-gnu/librt.a
copper: /usr/lib/x86_64-linux-gnu/libm.so
copper: _deps/webgpu-backend-wgpu-src/bin/linux-x86_64/libwgpu_native.so
copper: /usr/lib/x86_64-linux-gnu/libGLX.so
copper: /usr/lib/x86_64-linux-gnu/libOpenGL.so
copper: CMakeFiles/copper.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pabloxeo/Documents/copper/run/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable copper"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/copper.dir/link.txt --verbose=$(VERBOSE)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Copying '/home/pabloxeo/Documents/copper/run/_deps/webgpu-backend-wgpu-src/bin/linux-x86_64/libwgpu_native.so' to '\$$<TARGET_FILE_DIR:copper>'..."
	/usr/bin/cmake -E copy_if_different /home/pabloxeo/Documents/copper/run/_deps/webgpu-backend-wgpu-src/bin/linux-x86_64/libwgpu_native.so /home/pabloxeo/Documents/copper/run

# Rule to build all files generated by this target.
CMakeFiles/copper.dir/build: copper
.PHONY : CMakeFiles/copper.dir/build

CMakeFiles/copper.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/copper.dir/cmake_clean.cmake
.PHONY : CMakeFiles/copper.dir/clean

CMakeFiles/copper.dir/depend:
	cd /home/pabloxeo/Documents/copper/run && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pabloxeo/Documents/copper /home/pabloxeo/Documents/copper /home/pabloxeo/Documents/copper/run /home/pabloxeo/Documents/copper/run /home/pabloxeo/Documents/copper/run/CMakeFiles/copper.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/copper.dir/depend

