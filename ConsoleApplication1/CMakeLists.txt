cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-pthread")
project(server)                     #定义整个CMake的工程名
add_definitions("-Wall -std=c++11") # <= 新增的编译选项
file(GLOB SOURCE "./*.cpp")               #告诉CMake：工程的源文件在哪里？
#添加头文件搜索路径
include_directories(/usr/local/include)
#添加库文件搜索路径
link_directories(/usr/local/lib)
add_executable(${PROJECT_NAME} ${SOURCE})    #告诉CMake：我们要生成可执行文件（hello），并且源文件是由上面定义的
target_link_libraries(${PROJECT_NAME} "libboost_thread.a" "libboost_system.a" "libboost_date_time.a")
