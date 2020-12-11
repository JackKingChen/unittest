cmake_minimum_required(VERSION 3.2)

project(unittest)

add_compile_options(-std=c++11)

include_directories(${CMAKE_SOURCE_DIR}/libs/include
                    /usr/local/include
                    ./include)
link_directories(${CMAKE_SOURCE_DIR}/libs
                ../../ffmpegbuild/lib)


add_definitions(-DUSING_TRACE)

add_definitions(-DMIPCONFIG_SUPPORT_DUMPFILE)

set(FILES
    source/jsoncpp/json/json_reader.cpp
    source/jsoncpp/json/json_writer.cpp
    source/jsoncpp/json/json_value.cpp
    source/platform/linux/linux_console.cc
    source/platform/linux/linux_os.cc
    source/host_console.cc
    source/host_os.cc
    source/unittest.cc 
    source/main.cpp
    utest_hid/hid.cpp
    utest_hid/utest_hid.cpp
    utest_protocol_nd/NdVideoSession.cpp
    #utest_ffmpeg/utest_videotoolbox.cpp
    utest_protocol_nd/utest_protocol_nd.cpp
    #utest_coreaudio/utest_coreaudio.cpp
)

find_library(lib_quartz QuartzCore)
find_library(lib_coregraphics CoreGraphics)
find_library(lib_coreImage CoreImage)
find_library(lib_foundation Foundation)
find_library(lib_corefoundation CoreFoundation)
find_library(lib_iosurface IOSurface)

find_library(lib_coreaudio CoreAudio)
find_library(lib_AudioUnit AudioUnit)
find_library(lib_IOSurface IOSurface)
find_library(lib_Cocoa Cocoa)
find_library(lib_IOKit IOKit)
 
add_executable(unittest ${FILES})

target_link_libraries(unittest  emiplib jrtp pthread jthread avcodec swscale avutil ${lib_IOKit} ${lib_Cocoa} ${lib_IOSurface} ${lib_coreaudio} ${lib_AudioUnit} ${lib_coregraphics} ${lib_quartz} ${lib_coreImage} ${lib_iosurface} ${lib_corefoundation})
