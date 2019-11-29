cmake_minimum_required(VERSION 3.14)
project(lipc C)

set(CMAKE_C_STANDARD 11)

add_library(lipc SHARED sm_channel.c sm_channel.h util.c util.h map.c map.h)