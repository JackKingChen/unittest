cmake_minimum_required(VERSION 3.10)

project(unittest)

set(CMAKE_CXX_STANDARD 11)

set(FILES
    source/platform/linux/linux_console.cc
    source/platform/linux/linux_os.cc
    source/host_console.cc
    source/host_os.cc
    source/main.cc
    source/unittest.cc
    unittest_uci/utest_uci.cc
    )

include_directories(./include)
add_executable(unittest ${FILES})


target_link_libraries(unittest pthread ubox uci usb-1.0)

install(TARGETS unittest DESTINATION bin)