cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-pthread")
project(server)                     #定义整个CMake的工程名
add_definitions("-Wall -g -std=c++11") # <= 新增的编译选项
#添加头文件搜索路径
include_directories(/usr/local/include)
#添加库文件搜索路径
link_directories(/usr/local/lib)
#添加 netmodel子目录
add_subdirectory(NetModel)
#指定生成目标
add_executable(${PROJECT_NAME} main.cpp)
#添加链接库
target_link_libraries(${PROJECT_NAME} "libboost_thread.a" "libboost_system.a" "libboost_date_time.a" NetModellib)
