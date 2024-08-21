cmake_minimum_required(VERSION 3.28)
project(filetovideo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE DEBUG)

# set include directories
set(SrcDIR ${CMAKE_SOURCE_DIR}/src)
set(IncludeDIR ${CMAKE_SOURCE_DIR}/include)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)

# enable testing
enable_testing()

add_test(NAME encode_decode_error COMMAND ${CMAKE_SOURCE_DIR}/tests/encode_decode_error.sh ${CMAKE_BINARY_DIR}/filetovideo  ${CMAKE_SOURCE_DIR}/tests/data/lorem_ipsum_500p.txt)

include_directories(
        ${SrcDIR}
        ${IncludeDIR}
)

# get libraries

# avcodec
find_library(AVCODEC_LIBRARY avcodec)
if(NOT AVCODEC_LIBRARY)
    message(FATAL_ERROR "avcodec library not found")
endif()

# avformat
find_library(AVFORMAT_LIBRARY avformat)
if(NOT AVFORMAT_LIBRARY)
    message(FATAL_ERROR "avformat library not found")
endif()

# avutil
find_library(AVUTIL_LIBRARY avutil)
if(NOT AVUTIL_LIBRARY)
    message(FATAL_ERROR "avutil library not found")
endif()

# swscale
find_library(SWSCALE_LIBRARY swscale)
if(NOT SWSCALE_LIBRARY)
    message(FATAL_ERROR "swscale library not found")
endif()

file(GLOB_RECURSE SOURCE_FILES ${SrcDIR}/*.cpp ${SrcDIR}/*.c ${SrcDIR}/**/*.cpp ${SrcDIR}/**/*.c) 
file(GLOB_RECURSE HEADER_FILES ${SrcDIR}/*.h ${SrcDIR}/*.hpp ${SrcDIR}/**/*.h ${SrcDIR}/**/*.hpp)

add_executable(filetovideo ${CMAKE_SOURCE_DIR}/src/main.cpp ${SOURCE_FILES})

target_link_directories(filetovideo PUBLIC ${CMAKE_SOURCE_DIR}/src/)
target_link_libraries(filetovideo ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${FFMPEG_LIBRARY} ${SWSCALE_LIBRARY})

install(TARGETS filetovideo DESTINATION ${CMAKE_INSTALL_BINDIR})