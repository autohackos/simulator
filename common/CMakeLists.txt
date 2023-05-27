cmake_minimum_required(VERSION 3.16)
project(common)

set(CMAKE_CXX_STANDARD 17)

add_library(common SHARED ConfigurationParser.cpp)
