cmake_minimum_required(VERSION 3.30)
project(fsw_linux_homework_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(${PROJECT_NAME}
        src/client1.cpp
        src/Client.cpp
        include/ClientError.hpp
        include/Result.hpp)
