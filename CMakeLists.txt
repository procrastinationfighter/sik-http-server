cmake_minimum_required(VERSION 3.19)
project(SiK1)

set(CMAKE_CXX_STANDARD 17)

add_executable(SiK1 main.cpp Server.h server_utilities.cpp server_utilities.h HttpRequest.h exceptions.h HttpRequest.cpp Server.cpp)