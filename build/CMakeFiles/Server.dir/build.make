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
include CMakeFiles/Server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Server.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Server.dir/flags.make

CMakeFiles/Server.dir/Server.cpp.o: CMakeFiles/Server.dir/flags.make
CMakeFiles/Server.dir/Server.cpp.o: ../Server.cpp
CMakeFiles/Server.dir/Server.cpp.o: CMakeFiles/Server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hollow/Vs-Cpp/this-redis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Server.dir/Server.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Server.dir/Server.cpp.o -MF CMakeFiles/Server.dir/Server.cpp.o.d -o CMakeFiles/Server.dir/Server.cpp.o -c /home/hollow/Vs-Cpp/this-redis/Server.cpp

CMakeFiles/Server.dir/Server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Server.dir/Server.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hollow/Vs-Cpp/this-redis/Server.cpp > CMakeFiles/Server.dir/Server.cpp.i

CMakeFiles/Server.dir/Server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Server.dir/Server.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hollow/Vs-Cpp/this-redis/Server.cpp -o CMakeFiles/Server.dir/Server.cpp.s

CMakeFiles/Server.dir/Reader.cpp.o: CMakeFiles/Server.dir/flags.make
CMakeFiles/Server.dir/Reader.cpp.o: ../Reader.cpp
CMakeFiles/Server.dir/Reader.cpp.o: CMakeFiles/Server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hollow/Vs-Cpp/this-redis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Server.dir/Reader.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Server.dir/Reader.cpp.o -MF CMakeFiles/Server.dir/Reader.cpp.o.d -o CMakeFiles/Server.dir/Reader.cpp.o -c /home/hollow/Vs-Cpp/this-redis/Reader.cpp

CMakeFiles/Server.dir/Reader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Server.dir/Reader.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hollow/Vs-Cpp/this-redis/Reader.cpp > CMakeFiles/Server.dir/Reader.cpp.i

CMakeFiles/Server.dir/Reader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Server.dir/Reader.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hollow/Vs-Cpp/this-redis/Reader.cpp -o CMakeFiles/Server.dir/Reader.cpp.s

CMakeFiles/Server.dir/Writer.cpp.o: CMakeFiles/Server.dir/flags.make
CMakeFiles/Server.dir/Writer.cpp.o: ../Writer.cpp
CMakeFiles/Server.dir/Writer.cpp.o: CMakeFiles/Server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hollow/Vs-Cpp/this-redis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/Server.dir/Writer.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Server.dir/Writer.cpp.o -MF CMakeFiles/Server.dir/Writer.cpp.o.d -o CMakeFiles/Server.dir/Writer.cpp.o -c /home/hollow/Vs-Cpp/this-redis/Writer.cpp

CMakeFiles/Server.dir/Writer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Server.dir/Writer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hollow/Vs-Cpp/this-redis/Writer.cpp > CMakeFiles/Server.dir/Writer.cpp.i

CMakeFiles/Server.dir/Writer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Server.dir/Writer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hollow/Vs-Cpp/this-redis/Writer.cpp -o CMakeFiles/Server.dir/Writer.cpp.s

# Object files for target Server
Server_OBJECTS = \
"CMakeFiles/Server.dir/Server.cpp.o" \
"CMakeFiles/Server.dir/Reader.cpp.o" \
"CMakeFiles/Server.dir/Writer.cpp.o"

# External object files for target Server
Server_EXTERNAL_OBJECTS =

Server: CMakeFiles/Server.dir/Server.cpp.o
Server: CMakeFiles/Server.dir/Reader.cpp.o
Server: CMakeFiles/Server.dir/Writer.cpp.o
Server: CMakeFiles/Server.dir/build.make
Server: CMakeFiles/Server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hollow/Vs-Cpp/this-redis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable Server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Server.dir/build: Server
.PHONY : CMakeFiles/Server.dir/build

CMakeFiles/Server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Server.dir/clean

CMakeFiles/Server.dir/depend:
	cd /home/hollow/Vs-Cpp/this-redis/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hollow/Vs-Cpp/this-redis /home/hollow/Vs-Cpp/this-redis /home/hollow/Vs-Cpp/this-redis/build /home/hollow/Vs-Cpp/this-redis/build /home/hollow/Vs-Cpp/this-redis/build/CMakeFiles/Server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Server.dir/depend

