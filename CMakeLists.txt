# 指定使用 CMake 的最低版本号
cmake_minimum_required(VERSION 3.0)

# 设置项目名称
project(My-Muduo C CXX)

# 添加搜索路径
include_directories (
    ${PROJECT_SOURCE_DIR}/include/base
    ${PROJECT_SOURCE_DIR}/include/net
    ${PROJECT_SOURCE_DIR}/include/net/eventloop
    ${PROJECT_SOURCE_DIR}/include/net/poller
    ${PROJECT_SOURCE_DIR}/include/net/timer
    ${PROJECT_SOURCE_DIR}/include/net/tcp
    ${PROJECT_SOURCE_DIR}/include/logger
)

# 包含这些目录下的.cpp文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src/base SRC_BASE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net SRC_NET)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net/enentloop SRC_EVENTLOOP)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net/poller SRC_POLLER)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net/timer SRC_TIMER)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net/tcp SRC_TCP)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/logger SRC_LOGGER)

# mymuduo最终编译成so动态库 设置动态库的路径 放置项目根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set (CXX_FLAGS
    -g
    -Wall
    -std=c++11
)

add_library( My-Muduo SHARED 
    ${SRC_BASE}
    ${SRC_NET}
    ${SRC_EVENTLOOP}
    ${SRC_POLLER}
    ${SRC_TIMER}
    ${SRC_TCP}
    ${SRC_LOGGER}
)

# 目标动态库所需连接的库（这里需要连接libpthread.so）
target_link_libraries(My-Muduo pthread)
