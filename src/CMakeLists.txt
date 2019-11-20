cmake_minimum_required(VERSION 3.14)
project(smipc C)

set(CMAKE_C_STANDARD 11)

add_library(smipc SHARED library.c library.h util.c util.h)