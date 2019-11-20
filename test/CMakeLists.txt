cmake_minimum_required(VERSION 3.14)
project(smipctest C)

set(CMAKE_C_STANDARD 99)

set(LINK_DIR C:/Users/27251/CLionProjects/smipc)
include_directories(${LINK_DIR})
add_executable(smipctest main.c ${LINK_DIR}/util.h ${LINK_DIR}/util.c)
