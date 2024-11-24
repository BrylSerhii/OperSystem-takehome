cmake_minimum_required(VERSION 3.27)
project(OperSystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(OperSystem server.cpp
        client.cpp)
