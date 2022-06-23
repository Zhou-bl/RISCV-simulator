cmake_minimum_required(VERSION 3.16)
project(riscv_project)

set(CMAKE_CXX_STANDARD 14)

add_executable(code
        master.cpp)
