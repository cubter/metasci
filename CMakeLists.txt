cmake_minimum_required(VERSION 3.18.4)

project(metaSci LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(nlohmann_json_DIR ${PROJECT_SOURCE_DIR}/thirdparty/share/nlohmann_json)


include_directories(${PROJECT_SOURCE_DIR})
include_directories(AFTER SYSTEM ${PROJECT_SOURCE_DIR}/thirdparty/include/)

add_executable(metaSci crossref_parser.cpp)  

find_package(nlohmann_json REQUIRED)

target_link_libraries(metaSci PUBLIC nlohmann_json::nlohmann_json 
    -L${PROJECT_SOURCE_DIR}/thirdparty/lib/orc
    -L${PROJECT_SOURCE_DIR}/thirdparty/lib/protobuf
    -L${PROJECT_SOURCE_DIR}/thirdparty/lib/lz4
    -L${PROJECT_SOURCE_DIR}/thirdparty/lib/snappy
    -L${PROJECT_SOURCE_DIR}/thirdparty/lib/zstd
    -L${PROJECT_SOURCE_DIR}/thirdparty/lib/zlib
    -lorc
    -lprotoc
    -lprotobuf
    -lsnappy
    -llz4
    -lzstd
    -lz
    -lpthread)
        
target_compile_options(metaSci
	INTERFACE
	-Wall
	-Wextra
	-Wshadow
	-Wnon-virtual-dtor
	-Wold-style-cast
	-Wcast-align
	-Wunused
	-Woverloaded-virtual
	-Wpedantic
	-Wconversion
	-Wsign-conversion
	-Wnull-dereference
	-Wdouble-promotion
	-Wformat=2
	-Wreorder
	-O2)

