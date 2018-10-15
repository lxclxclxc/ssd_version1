cmake_minimum_required(VERSION 3.7)
project(SSDsim_origin)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES avlTree.c avlTree.h flash.c flash.h initialize.h initialize.c pagemap.h pagemap.c ssd.h ssd.c)
add_executable(SSDsim_origin ${SOURCE_FILES})