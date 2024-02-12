# 查找 Boost 库并设置变量
find_package(Boost COMPONENTS REQUIRED)
if (Boost_FOUND)
    message(STATUS "Found Boost ${Boost_VERSION} at ${Boost_INCLUDE_DIRS}")

    # 将 Boost 头文件目录添加到项目中
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost library not found")
endif ()

# 创建名为 libconet 的静态库
add_library(libspnet)

# 使用通配符获取头文件和源文件列表
file(GLOB_RECURSE LIB_HEADERS "*.h")
file(GLOB_RECURSE LIB_SOURCES "*.cpp")

# message("Found head file: ${LIB_HEADERS}")

# 向库添加头文件和源文件
target_sources(libspnet
    PUBLIC
        ${LIB_HEADERS}

    PRIVATE
        ${LIB_SOURCES}
)

target_include_directories(libspnet
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# 将依赖的库链接到 libconet
target_link_libraries(libspnet 
PRIVATE 
${Boost_LIBRARIES} 
yaml-cpp 
pthread 
dl
)