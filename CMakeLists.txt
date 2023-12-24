cmake_minimum_required(VERSION 3.22)
project(message_router)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(message_router ${SOURCES})
