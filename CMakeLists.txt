cmake_minimum_required(VERSION 3.20)

project(
    patternia
    VERSION 0.1.0
    DESCRIPTION "Match pattern lib for modern C++"
    LANGUAGES CXX
)

# 全局开启 C++20
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(patternia src/main.cpp)

target_include_directories(patternia PRIVATE include)
