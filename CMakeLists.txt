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
    #utest_protocol_nd/NdVideoSession.cpp
    #utest_protocol_nd/utest_protocol_nd.cpp
    utest_coreaudio/utest_coreaudio.cpp
)

find_library(lib_quartz QuartzCore)
find_library(lib_coregraphics CoreGraphics)
find_library(lib_coreImage CoreImage)
find_library(lib_foundation Foundation)
find_library(lib_corefoundation CoreFoundation)
find_library(lib_iosurface IOSurface)

find_library(lib_coreaudio CoreAudio)
find_library(lib_AudioUnit AudioUnit)
 
add_executable(unittest ${FILES})

target_link_libraries(unittest  emiplib jrtp pthread jthread avcodec swscale avutil SDL2 ${lib_coreaudio} ${lib_AudioUnit} ${lib_coregraphics} ${lib_quartz} ${lib_coreImage} ${lib_iosurface} ${lib_corefoundation})