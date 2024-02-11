cmake_minimum_required(VERSION 3.10)

# 设置全局编译选项
set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_options(-O3 -g -ggdb -Wno-unused-variable -Wno-deprecated -Wno-unused-function -Werror -Wno-builtin-macro-redefined -Wno-deprecated-declarations)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(lim_webserver)

set(CMAKE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output)
# 设置库的输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加子目录
add_subdirectory(test)
add_subdirectory(spike)
add_subdirectory(bench)
add_subdirectory(examples)