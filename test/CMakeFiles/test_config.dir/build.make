# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/book/Webserver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/book/Webserver

# Include any dependencies generated for this target.
include test/CMakeFiles/test_config.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test_config.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test_config.dir/flags.make

test/CMakeFiles/test_config.dir/test_config.cpp.o: test/CMakeFiles/test_config.dir/flags.make
test/CMakeFiles/test_config.dir/test_config.cpp.o: test/test_config.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/test_config.dir/test_config.cpp.o"
	cd /home/book/Webserver/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_config.dir/test_config.cpp.o -c /home/book/Webserver/test/test_config.cpp

test/CMakeFiles/test_config.dir/test_config.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_config.dir/test_config.cpp.i"
	cd /home/book/Webserver/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/test/test_config.cpp > CMakeFiles/test_config.dir/test_config.cpp.i

test/CMakeFiles/test_config.dir/test_config.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_config.dir/test_config.cpp.s"
	cd /home/book/Webserver/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/test/test_config.cpp -o CMakeFiles/test_config.dir/test_config.cpp.s

test/CMakeFiles/test_config.dir/test_config.cpp.o.requires:

.PHONY : test/CMakeFiles/test_config.dir/test_config.cpp.o.requires

test/CMakeFiles/test_config.dir/test_config.cpp.o.provides: test/CMakeFiles/test_config.dir/test_config.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/test_config.dir/build.make test/CMakeFiles/test_config.dir/test_config.cpp.o.provides.build
.PHONY : test/CMakeFiles/test_config.dir/test_config.cpp.o.provides

test/CMakeFiles/test_config.dir/test_config.cpp.o.provides.build: test/CMakeFiles/test_config.dir/test_config.cpp.o


# Object files for target test_config
test_config_OBJECTS = \
"CMakeFiles/test_config.dir/test_config.cpp.o"

# External object files for target test_config
test_config_EXTERNAL_OBJECTS =

output/test_config: test/CMakeFiles/test_config.dir/test_config.cpp.o
output/test_config: test/CMakeFiles/test_config.dir/build.make
output/test_config: lib/liblibconet.a
output/test_config: test/CMakeFiles/test_config.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../output/test_config"
	cd /home/book/Webserver/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_config.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test_config.dir/build: output/test_config

.PHONY : test/CMakeFiles/test_config.dir/build

test/CMakeFiles/test_config.dir/requires: test/CMakeFiles/test_config.dir/test_config.cpp.o.requires

.PHONY : test/CMakeFiles/test_config.dir/requires

test/CMakeFiles/test_config.dir/clean:
	cd /home/book/Webserver/test && $(CMAKE_COMMAND) -P CMakeFiles/test_config.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test_config.dir/clean

test/CMakeFiles/test_config.dir/depend:
	cd /home/book/Webserver && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/book/Webserver /home/book/Webserver/test /home/book/Webserver /home/book/Webserver/test /home/book/Webserver/test/CMakeFiles/test_config.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test_config.dir/depend

