cmake_minimum_required(VERSION 3.16)
project(canbus_simulator)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${PROJECT_SOURCE_DIR}/controller)
add_subdirectory(${PROJECT_SOURCE_DIR}/console)
