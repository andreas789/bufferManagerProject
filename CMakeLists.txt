cmake_minimum_required(VERSION 3.28)
project(bufferManagerProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(bufferManagerProject main.cpp
        src/SmartArrayMemoryManager.h
        src/SmartArrayMemoryManager.cpp
)
