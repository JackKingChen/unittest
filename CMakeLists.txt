cmake_minimum_required(VERSION 3.2)

project(unittest)

add_compile_options(-std=c++11)

include_directories(${CMAKE_SOURCE_DIR}/libs/include
                    /usr/local/include
                    ./include)
link_directories(${CMAKE_SOURCE_DIR}/libs
                /usr/local/lib)


add_definitions(-DUSING_TRACE)

set(FILES
    source/platform/linux/linux_console.cc
    source/platform/linux/linux_os.cc
    source/host_console.cc
    source/host_os.cc
    source/unittest.cc 
    source/main.cpp
)

 
add_executable(unittest ${FILES})

# target_link_libraries(unittest)