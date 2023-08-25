# 查找 Boost 库并设置变量
find_package(Boost COMPONENTS REQUIRED)
if (Boost_FOUND)
    message(STATUS "Found Boost ${Boost_VERSION} at ${Boost_INCLUDE_DIRS}")

    # 将 Boost 头文件目录添加到项目中
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost library not found")
endif ()

# 查找 yaml-cpp 库
find_package(yaml-cpp REQUIRED)

if (yaml-cpp_FOUND)
    message(STATUS "Found yaml-cpp ${YAML_CPP_VERSION}")
else ()
    message(FATAL_ERROR "yaml-cpp library not found")
endif ()

# 添加 src 目录下的所有源文件到变量
aux_source_directory(. LIBCONET_SRC_LIST)

# 创建名为 libconet 的静态库
add_library(libconet STATIC ${LIBCONET_SRC_LIST})

# 将依赖的库链接到 libconet
target_link_libraries(libconet PRIVATE ${Boost_LIBRARIES} yaml-cpp pthread dl)