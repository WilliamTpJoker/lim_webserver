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
include src/CMakeFiles/libconet.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/libconet.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/libconet.dir/flags.make

src/CMakeFiles/libconet.dir/config.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/config.cpp.o: src/config.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/libconet.dir/config.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/config.cpp.o -c /home/book/Webserver/src/config.cpp

src/CMakeFiles/libconet.dir/config.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/config.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/config.cpp > CMakeFiles/libconet.dir/config.cpp.i

src/CMakeFiles/libconet.dir/config.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/config.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/config.cpp -o CMakeFiles/libconet.dir/config.cpp.s

src/CMakeFiles/libconet.dir/config.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/config.cpp.o.requires

src/CMakeFiles/libconet.dir/config.cpp.o.provides: src/CMakeFiles/libconet.dir/config.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/config.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/config.cpp.o.provides

src/CMakeFiles/libconet.dir/config.cpp.o.provides.build: src/CMakeFiles/libconet.dir/config.cpp.o


src/CMakeFiles/libconet.dir/fd_manager.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/fd_manager.cpp.o: src/fd_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/libconet.dir/fd_manager.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/fd_manager.cpp.o -c /home/book/Webserver/src/fd_manager.cpp

src/CMakeFiles/libconet.dir/fd_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/fd_manager.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/fd_manager.cpp > CMakeFiles/libconet.dir/fd_manager.cpp.i

src/CMakeFiles/libconet.dir/fd_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/fd_manager.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/fd_manager.cpp -o CMakeFiles/libconet.dir/fd_manager.cpp.s

src/CMakeFiles/libconet.dir/fd_manager.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/fd_manager.cpp.o.requires

src/CMakeFiles/libconet.dir/fd_manager.cpp.o.provides: src/CMakeFiles/libconet.dir/fd_manager.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/fd_manager.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/fd_manager.cpp.o.provides

src/CMakeFiles/libconet.dir/fd_manager.cpp.o.provides.build: src/CMakeFiles/libconet.dir/fd_manager.cpp.o


src/CMakeFiles/libconet.dir/fiber.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/fiber.cpp.o: src/fiber.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/CMakeFiles/libconet.dir/fiber.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/fiber.cpp.o -c /home/book/Webserver/src/fiber.cpp

src/CMakeFiles/libconet.dir/fiber.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/fiber.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/fiber.cpp > CMakeFiles/libconet.dir/fiber.cpp.i

src/CMakeFiles/libconet.dir/fiber.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/fiber.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/fiber.cpp -o CMakeFiles/libconet.dir/fiber.cpp.s

src/CMakeFiles/libconet.dir/fiber.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/fiber.cpp.o.requires

src/CMakeFiles/libconet.dir/fiber.cpp.o.provides: src/CMakeFiles/libconet.dir/fiber.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/fiber.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/fiber.cpp.o.provides

src/CMakeFiles/libconet.dir/fiber.cpp.o.provides.build: src/CMakeFiles/libconet.dir/fiber.cpp.o


src/CMakeFiles/libconet.dir/hook.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/hook.cpp.o: src/hook.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/CMakeFiles/libconet.dir/hook.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/hook.cpp.o -c /home/book/Webserver/src/hook.cpp

src/CMakeFiles/libconet.dir/hook.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/hook.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/hook.cpp > CMakeFiles/libconet.dir/hook.cpp.i

src/CMakeFiles/libconet.dir/hook.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/hook.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/hook.cpp -o CMakeFiles/libconet.dir/hook.cpp.s

src/CMakeFiles/libconet.dir/hook.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/hook.cpp.o.requires

src/CMakeFiles/libconet.dir/hook.cpp.o.provides: src/CMakeFiles/libconet.dir/hook.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/hook.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/hook.cpp.o.provides

src/CMakeFiles/libconet.dir/hook.cpp.o.provides.build: src/CMakeFiles/libconet.dir/hook.cpp.o


src/CMakeFiles/libconet.dir/io_manager.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/io_manager.cpp.o: src/io_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/CMakeFiles/libconet.dir/io_manager.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/io_manager.cpp.o -c /home/book/Webserver/src/io_manager.cpp

src/CMakeFiles/libconet.dir/io_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/io_manager.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/io_manager.cpp > CMakeFiles/libconet.dir/io_manager.cpp.i

src/CMakeFiles/libconet.dir/io_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/io_manager.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/io_manager.cpp -o CMakeFiles/libconet.dir/io_manager.cpp.s

src/CMakeFiles/libconet.dir/io_manager.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/io_manager.cpp.o.requires

src/CMakeFiles/libconet.dir/io_manager.cpp.o.provides: src/CMakeFiles/libconet.dir/io_manager.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/io_manager.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/io_manager.cpp.o.provides

src/CMakeFiles/libconet.dir/io_manager.cpp.o.provides.build: src/CMakeFiles/libconet.dir/io_manager.cpp.o


src/CMakeFiles/libconet.dir/log.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/log.cpp.o: src/log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/CMakeFiles/libconet.dir/log.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/log.cpp.o -c /home/book/Webserver/src/log.cpp

src/CMakeFiles/libconet.dir/log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/log.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/log.cpp > CMakeFiles/libconet.dir/log.cpp.i

src/CMakeFiles/libconet.dir/log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/log.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/log.cpp -o CMakeFiles/libconet.dir/log.cpp.s

src/CMakeFiles/libconet.dir/log.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/log.cpp.o.requires

src/CMakeFiles/libconet.dir/log.cpp.o.provides: src/CMakeFiles/libconet.dir/log.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/log.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/log.cpp.o.provides

src/CMakeFiles/libconet.dir/log.cpp.o.provides.build: src/CMakeFiles/libconet.dir/log.cpp.o


src/CMakeFiles/libconet.dir/main.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/main.cpp.o: src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object src/CMakeFiles/libconet.dir/main.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/main.cpp.o -c /home/book/Webserver/src/main.cpp

src/CMakeFiles/libconet.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/main.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/main.cpp > CMakeFiles/libconet.dir/main.cpp.i

src/CMakeFiles/libconet.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/main.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/main.cpp -o CMakeFiles/libconet.dir/main.cpp.s

src/CMakeFiles/libconet.dir/main.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/main.cpp.o.requires

src/CMakeFiles/libconet.dir/main.cpp.o.provides: src/CMakeFiles/libconet.dir/main.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/main.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/main.cpp.o.provides

src/CMakeFiles/libconet.dir/main.cpp.o.provides.build: src/CMakeFiles/libconet.dir/main.cpp.o


src/CMakeFiles/libconet.dir/scheduler.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/scheduler.cpp.o: src/scheduler.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object src/CMakeFiles/libconet.dir/scheduler.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/scheduler.cpp.o -c /home/book/Webserver/src/scheduler.cpp

src/CMakeFiles/libconet.dir/scheduler.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/scheduler.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/scheduler.cpp > CMakeFiles/libconet.dir/scheduler.cpp.i

src/CMakeFiles/libconet.dir/scheduler.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/scheduler.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/scheduler.cpp -o CMakeFiles/libconet.dir/scheduler.cpp.s

src/CMakeFiles/libconet.dir/scheduler.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/scheduler.cpp.o.requires

src/CMakeFiles/libconet.dir/scheduler.cpp.o.provides: src/CMakeFiles/libconet.dir/scheduler.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/scheduler.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/scheduler.cpp.o.provides

src/CMakeFiles/libconet.dir/scheduler.cpp.o.provides.build: src/CMakeFiles/libconet.dir/scheduler.cpp.o


src/CMakeFiles/libconet.dir/thread.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/thread.cpp.o: src/thread.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object src/CMakeFiles/libconet.dir/thread.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/thread.cpp.o -c /home/book/Webserver/src/thread.cpp

src/CMakeFiles/libconet.dir/thread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/thread.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/thread.cpp > CMakeFiles/libconet.dir/thread.cpp.i

src/CMakeFiles/libconet.dir/thread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/thread.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/thread.cpp -o CMakeFiles/libconet.dir/thread.cpp.s

src/CMakeFiles/libconet.dir/thread.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/thread.cpp.o.requires

src/CMakeFiles/libconet.dir/thread.cpp.o.provides: src/CMakeFiles/libconet.dir/thread.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/thread.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/thread.cpp.o.provides

src/CMakeFiles/libconet.dir/thread.cpp.o.provides.build: src/CMakeFiles/libconet.dir/thread.cpp.o


src/CMakeFiles/libconet.dir/timer.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/timer.cpp.o: src/timer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object src/CMakeFiles/libconet.dir/timer.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/timer.cpp.o -c /home/book/Webserver/src/timer.cpp

src/CMakeFiles/libconet.dir/timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/timer.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/timer.cpp > CMakeFiles/libconet.dir/timer.cpp.i

src/CMakeFiles/libconet.dir/timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/timer.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/timer.cpp -o CMakeFiles/libconet.dir/timer.cpp.s

src/CMakeFiles/libconet.dir/timer.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/timer.cpp.o.requires

src/CMakeFiles/libconet.dir/timer.cpp.o.provides: src/CMakeFiles/libconet.dir/timer.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/timer.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/timer.cpp.o.provides

src/CMakeFiles/libconet.dir/timer.cpp.o.provides.build: src/CMakeFiles/libconet.dir/timer.cpp.o


src/CMakeFiles/libconet.dir/util.cpp.o: src/CMakeFiles/libconet.dir/flags.make
src/CMakeFiles/libconet.dir/util.cpp.o: src/util.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object src/CMakeFiles/libconet.dir/util.cpp.o"
	cd /home/book/Webserver/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libconet.dir/util.cpp.o -c /home/book/Webserver/src/util.cpp

src/CMakeFiles/libconet.dir/util.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libconet.dir/util.cpp.i"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/book/Webserver/src/util.cpp > CMakeFiles/libconet.dir/util.cpp.i

src/CMakeFiles/libconet.dir/util.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libconet.dir/util.cpp.s"
	cd /home/book/Webserver/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/book/Webserver/src/util.cpp -o CMakeFiles/libconet.dir/util.cpp.s

src/CMakeFiles/libconet.dir/util.cpp.o.requires:

.PHONY : src/CMakeFiles/libconet.dir/util.cpp.o.requires

src/CMakeFiles/libconet.dir/util.cpp.o.provides: src/CMakeFiles/libconet.dir/util.cpp.o.requires
	$(MAKE) -f src/CMakeFiles/libconet.dir/build.make src/CMakeFiles/libconet.dir/util.cpp.o.provides.build
.PHONY : src/CMakeFiles/libconet.dir/util.cpp.o.provides

src/CMakeFiles/libconet.dir/util.cpp.o.provides.build: src/CMakeFiles/libconet.dir/util.cpp.o


# Object files for target libconet
libconet_OBJECTS = \
"CMakeFiles/libconet.dir/config.cpp.o" \
"CMakeFiles/libconet.dir/fd_manager.cpp.o" \
"CMakeFiles/libconet.dir/fiber.cpp.o" \
"CMakeFiles/libconet.dir/hook.cpp.o" \
"CMakeFiles/libconet.dir/io_manager.cpp.o" \
"CMakeFiles/libconet.dir/log.cpp.o" \
"CMakeFiles/libconet.dir/main.cpp.o" \
"CMakeFiles/libconet.dir/scheduler.cpp.o" \
"CMakeFiles/libconet.dir/thread.cpp.o" \
"CMakeFiles/libconet.dir/timer.cpp.o" \
"CMakeFiles/libconet.dir/util.cpp.o"

# External object files for target libconet
libconet_EXTERNAL_OBJECTS =

lib/liblibconet.a: src/CMakeFiles/libconet.dir/config.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/fd_manager.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/fiber.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/hook.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/io_manager.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/log.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/main.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/scheduler.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/thread.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/timer.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/util.cpp.o
lib/liblibconet.a: src/CMakeFiles/libconet.dir/build.make
lib/liblibconet.a: src/CMakeFiles/libconet.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/book/Webserver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Linking CXX static library ../lib/liblibconet.a"
	cd /home/book/Webserver/src && $(CMAKE_COMMAND) -P CMakeFiles/libconet.dir/cmake_clean_target.cmake
	cd /home/book/Webserver/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libconet.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/libconet.dir/build: lib/liblibconet.a

.PHONY : src/CMakeFiles/libconet.dir/build

src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/config.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/fd_manager.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/fiber.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/hook.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/io_manager.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/log.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/main.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/scheduler.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/thread.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/timer.cpp.o.requires
src/CMakeFiles/libconet.dir/requires: src/CMakeFiles/libconet.dir/util.cpp.o.requires

.PHONY : src/CMakeFiles/libconet.dir/requires

src/CMakeFiles/libconet.dir/clean:
	cd /home/book/Webserver/src && $(CMAKE_COMMAND) -P CMakeFiles/libconet.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/libconet.dir/clean

src/CMakeFiles/libconet.dir/depend:
	cd /home/book/Webserver && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/book/Webserver /home/book/Webserver/src /home/book/Webserver /home/book/Webserver/src /home/book/Webserver/src/CMakeFiles/libconet.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/libconet.dir/depend

