cmake_minimum_required(VERSION 3.10)

project(MyProject)

# 指定 C++ 标准
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED True)


# # 遍历每个源文件并创建一个可执行文件
# foreach(SOURCE_FILE ${SOURCES})
#     # 获取源文件的文件名（不带扩展名）
#     get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)

#     # 为每个源文件添加可执行文件
#     add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
# endforeach()
# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 添加主应用程序的可执行文件
add_executable(ServerApp Server.cpp Reader.cpp Writer.cpp)

# 添加单独的 Client 应用程序的可执行文件
add_executable(ClientApp Client.cpp)
