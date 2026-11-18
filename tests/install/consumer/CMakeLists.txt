cmake_minimum_required(VERSION 3.14)
project(ptn_install_consumer LANGUAGES CXX)

find_package(patternia CONFIG REQUIRED)

add_executable(ptn_install_consumer main.cpp)
target_link_libraries(ptn_install_consumer PRIVATE patternia::patternia)
