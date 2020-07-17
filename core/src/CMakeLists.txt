cmake_minimum_required(VERSION 3.14)
project(smipc C)

set(CMAKE_C_STANDARD 11)

find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})

add_library(smipc SHARED sm_channel.c sm_channel.h util.c util.h map.c map.h org_luncert_smipc_Smipc.h jni_impl.c)