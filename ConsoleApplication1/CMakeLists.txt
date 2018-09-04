cmake_minimum_required(VERSION 2.8)
PROJECT(server)                     #定义整个CMake的工程名
add_definitions("-Wall -std=c++11") # <= 新增的编译选项
FILE(GLOB SOURCE "./*.cpp")         #告诉CMake：工程的源文件在哪里？
ADD_EXECUTABLE(server ${SOURCE})    #告诉CMake：我们要生成可执行文件（hello），并且源文件是由上面定义的