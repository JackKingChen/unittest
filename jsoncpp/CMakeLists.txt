cmake_minimum_required(VERSION 3.2)

project(jsoncpp)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI})

add_compile_options(-std=c++11 -fPIC)


include_directories("./json")

set(SRC_HEADER
    json/autolink.h
    json/config.h
    json/features.h
    json/forwards.h
    json/json.h
    json/reader.h
    json/writer.h
    json/value.h)

set(SRC_LIST
    json/json_reader.cpp
    json/json_writer.cpp
    json/json_value.cpp)

add_library(jsoncpp STATIC ${SRC_LIST} ${SRC_HEADER})

#target_link_libraries(jsoncpp)

message("Begin install ${PROJECT} header files" )
install(FILES "${SRC_HEADER}" DESTINATION "${CMAKE_SOURCE_DIR}/libs/include/jsoncpp")
file(COPY ${SRC_HEADER} DESTINATION "${CMAKE_SOURCE_DIR}/../../../libs/include/jsoncpp")
message("End install header files ")

