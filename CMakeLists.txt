cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")

project(SequentialLogicSolver)


add_executable(main "src/main.cpp" "src/header.cpp")

target_include_directories(main PRIVATE "include")
