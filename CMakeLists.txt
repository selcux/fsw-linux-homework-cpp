cmake_minimum_required(VERSION 3.30)
project(fsw_linux_homework_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Common source files
set(COMMON_SOURCES
        include/ClientError.hpp
        include/Result.hpp
        src/Client.cpp)

add_executable(client1
        src/client1.cpp
        ${COMMON_SOURCES})

add_executable(client2
        src/client2.cpp
        ${COMMON_SOURCES})
