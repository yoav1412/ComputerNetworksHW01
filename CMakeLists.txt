cmake_minimum_required(VERSION 3.8)
project(untitled)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES server.c client.c)
set(HEADER_FILES server.h client.h)
add_executable(untitled ${SOURCE_FILES})