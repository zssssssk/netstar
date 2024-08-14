cmake_minimum_required(VERSION 3.5)
project(netstar)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC -fsanitize=address")

aux_source_directory(. SRC)
add_library(netstar SHARED ${SRC})