cmake_minimum_required(VERSION 3.20)
project(message_router)

set(CMAKE_CXX_STANDARD 23)


add_executable(
        message_router src/main.cpp
)

target_link_libraries(message_router -luuid)
