project(Core CXX)
cmake_minimum_required(VERSION 2.8)
#Add all needed files
include_directories(. ${PROJECT_3RD_LOC}/include)
file(GLOB_RECURSE MYSources *.cpp)
message(STATUS "CXX files - ${MYSources}")
#Compile staticly
add_library(Core STATIC ${MYSources} Process.cpp ChildProcess.cpp Pipe.cpp Future.h AutoResetEvent.cpp AutoResetEvent.h)
