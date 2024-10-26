cmake_minimum_required(VERSION 3.28)
project(aht10)


add_executable(aht10
    linux/main.cpp
)

message(${CMAKE_SOURCE_DIR}/include)

target_include_directories(aht10
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
