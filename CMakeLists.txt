cmake_minimum_required(VERSION 3.10)

project(MyProject)

# 指定 C++ 标准
# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

# 源文件列表
set(SOURCES
    Client.cpp
    Server.cpp
    d-Client.cpp
    d-Server.cpp
)

# 遍历每个源文件并创建一个可执行文件
foreach(SOURCE_FILE ${SOURCES})
    # 获取源文件的文件名（不带扩展名）
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)

    # 为每个源文件添加可执行文件
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
endforeach()
