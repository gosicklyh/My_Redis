cmake_minimum_required(VERSION 3.10)

project(MyProject)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)



# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 添加主应用程序的可执行文件
add_executable(ServerApp Server.cpp io_Read&Writer.cpp do_someing.cpp)

add_executable(D-ServerApp D-Server.cpp)

# 添加单独的 Client 应用程序的可执行文件
add_executable(ClientApp Client.cpp)

add_executable(D-ClientApp D-Client.cpp)