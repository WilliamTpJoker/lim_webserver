cmake_minimum_required(VERSION 3.10)
project(lim_webserver)

# 设置全局编译选项
set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_options(-O0 -g -ggdb -Wno-unused-variable)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 添加项目头文件目录
include_directories(include)

# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)
# 设置库的输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加子目录
add_subdirectory(src)
add_subdirectory(test)
