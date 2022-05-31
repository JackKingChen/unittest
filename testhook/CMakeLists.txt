cmake_minimum_required(VERSION 3.2)

project(speed)

add_compile_options(-std=c++11)

add_definitions(-DUSING_TRACE)

set(FILES
   speed.c
)

add_library(speed SHARED ${FILES})
target_link_libraries(unittest msquic)