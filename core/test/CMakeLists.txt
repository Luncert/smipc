cmake_minimum_required(VERSION 3.14)
project(smipctest C)

set(CMAKE_C_STANDARD 99)

set(LINK_DIR ../src)
include_directories(${LINK_DIR})
add_executable(smipctest main.c ${LINK_DIR}/sm_channel.c ${LINK_DIR}/sm_channel.h ${LINK_DIR}/util.c ${LINK_DIR}/util.h  ${LINK_DIR}/map.c ${LINK_DIR}/map.h crc.c crc.h)

