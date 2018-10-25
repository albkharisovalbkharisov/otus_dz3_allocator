cmake_minimum_required(VERSION 3.5)

project(06.allocator)

add_executable(allocator allocator.cpp)

set_target_properties(allocator PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
        )
