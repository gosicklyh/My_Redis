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
CMAKE_SOURCE_DIR = /home/hollow/Vs-Cpp/this-redis

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hollow/Vs-Cpp/this-redis/build

# Include any dependencies generated for this target.
include CMakeFiles/s-Client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/s-Client.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/s-Client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/s-Client.dir/flags.make

CMakeFiles/s-Client.dir/s-Client.cpp.o: CMakeFiles/s-Client.dir/flags.make
CMakeFiles/s-Client.dir/s-Client.cpp.o: ../s-Client.cpp
CMakeFiles/s-Client.dir/s-Client.cpp.o: CMakeFiles/s-Client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hollow/Vs-Cpp/this-redis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/s-Client.dir/s-Client.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/s-Client.dir/s-Client.cpp.o -MF CMakeFiles/s-Client.dir/s-Client.cpp.o.d -o CMakeFiles/s-Client.dir/s-Client.cpp.o -c /home/hollow/Vs-Cpp/this-redis/s-Client.cpp

CMakeFiles/s-Client.dir/s-Client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/s-Client.dir/s-Client.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hollow/Vs-Cpp/this-redis/s-Client.cpp > CMakeFiles/s-Client.dir/s-Client.cpp.i

CMakeFiles/s-Client.dir/s-Client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/s-Client.dir/s-Client.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hollow/Vs-Cpp/this-redis/s-Client.cpp -o CMakeFiles/s-Client.dir/s-Client.cpp.s

# Object files for target s-Client
s__Client_OBJECTS = \
"CMakeFiles/s-Client.dir/s-Client.cpp.o"

# External object files for target s-Client
s__Client_EXTERNAL_OBJECTS =

s-Client: CMakeFiles/s-Client.dir/s-Client.cpp.o
s-Client: CMakeFiles/s-Client.dir/build.make
s-Client: CMakeFiles/s-Client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hollow/Vs-Cpp/this-redis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable s-Client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/s-Client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/s-Client.dir/build: s-Client
.PHONY : CMakeFiles/s-Client.dir/build

CMakeFiles/s-Client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/s-Client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/s-Client.dir/clean

CMakeFiles/s-Client.dir/depend:
	cd /home/hollow/Vs-Cpp/this-redis/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hollow/Vs-Cpp/this-redis /home/hollow/Vs-Cpp/this-redis /home/hollow/Vs-Cpp/this-redis/build /home/hollow/Vs-Cpp/this-redis/build /home/hollow/Vs-Cpp/this-redis/build/CMakeFiles/s-Client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/s-Client.dir/depend

