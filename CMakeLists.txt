cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER ${GCC_ENV_DIR})
set(CMAKE_CXX_COMPILER ${GPP_ENV_DIR})

project(socket_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(mySocket_exe main.cpp)

target_link_libraries(mySocket_exe pthread)