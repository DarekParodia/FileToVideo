cmake_minimum_required(VERSION 3.28)
project(filetovideo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE DEBUG)

# set include directories
set(SrcDIR ${CMAKE_SOURCE_DIR}/src)
set(IncludeDIR ${CMAKE_SOURCE_DIR}/include)

include_directories(
        ${SrcDIR}
        ${IncludeDIR}
)

file(GLOB_RECURSE SOURCE_FILES ${SrcDIR}/*.cpp ${SrcDIR}/*.c ${SrcDIR}/**/*.cpp ${SrcDIR}/**/*.c) 
file(GLOB_RECURSE HEADER_FILES ${SrcDIR}/*.h ${SrcDIR}/*.hpp ${SrcDIR}/**/*.h ${SrcDIR}/**/*.hpp)

add_executable(filetovideo ${CMAKE_SOURCE_DIR}/src/main.cpp ${SOURCE_FILES})

target_link_directories(filetovideo PUBLIC ${CMAKE_SOURCE_DIR}/src/)