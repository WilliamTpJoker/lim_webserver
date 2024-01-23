include_directories(../spike)

file(GLOB_RECURSE CPP_SRC_LIST *.cpp)

foreach(CPP_FILE ${CPP_SRC_LIST})
    # 提取文件名作为可执行文件名
    get_filename_component(target_name ${CPP_FILE} NAME_WE)

    message(STATUS "找到测试文件：${CPP_FILE}")

    # 添加可执行文件目标
    add_executable(${target_name} ${CPP_FILE})

    # 设置可执行文件的输出路径
    set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/bench)

    # 链接 libspnet
    target_link_libraries(${target_name} libspnet)
endforeach()