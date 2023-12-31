cmake_minimum_required(VERSION 3.2)

project(unittest)

add_compile_options(-std=c++11)

include_directories(${CMAKE_SOURCE_DIR}/libs/include
                    /usr/local/include
                    ./include
                    ./source/include)
link_directories(${CMAKE_SOURCE_DIR}/libs
                /usr/local/lib)


add_definitions(-DUSING_TRACE)



if (CMAKE_SYSTEM_NAME MATCHES "Windows")
set(FILES
    source/platform/window/windows_console.cc
    source/platform/window/windows_os.cc
    source/host_console.cc
    source/host_os.cc
    source/unittest.cc
    source/main.cpp
    source/msquicclient.cpp
    source/msquicserver.cpp
    source/msquicstream.cpp
    source/cloudphonenetperf.cpp
    source/test_quic_client.cpp
)
else()
set(FILES
    source/platform/linux/linux_console.cc
    source/platform/linux/linux_os.cc
    source/host_console.cc
    source/host_os.cc
    source/unittest.cc
    source/main.cpp  
    source/cloudphonenetperf.cpp
    source/CaeQuicSocket.cpp
    source/CaeQuicServer.cpp
    source/test_cae_perf.cpp
)

endif()

link_directories(${CMAKE_SOURCE_DIR}/libs)
				
#add_subdirectory(jsoncpp)
add_executable(unittest ${FILES})
target_link_libraries(unittest msquic)
