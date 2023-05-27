cmake_minimum_required(VERSION 3.16)
project(canbus_simulator)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${PROJECT_SOURCE_DIR}/common)

add_executable(console console/main.cpp)
target_link_libraries(console common)

add_executable(controller controller/main.cpp)
target_link_libraries(controller common)
