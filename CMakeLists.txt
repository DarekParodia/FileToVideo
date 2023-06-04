cmake_minimum_required(VERSION 2.9)
set(CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -ggdb -Wall")
set(OpenCV_DIR /usr/local/opencv-3.4.0/share/OpenCV)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

project (filetovideo)
find_package(OpenCV REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -Wall")

include_directories(.)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(filetovideo main.cpp libattopng.c generator.cpp vidgen.cpp) 
target_link_libraries(filetovideo ${OpenCV_LIBS})   