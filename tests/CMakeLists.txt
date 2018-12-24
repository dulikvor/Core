project(sweetPyTests CXX)
cmake_minimum_required(VERSION 3.0)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_COLOR_MAKEFILE ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Default build type 'Debug'")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE )
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_DEBUG_POSTFIX)
    set(CORE_DEBUG_POSTFIX d)
endif()

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_DIR}/bin)
set(CMAKE_BINARY_DIR ${PROJECT_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_DIR}/bin)

include_directories(. ..  ${PROJECT_DIR} ${CORE_3RD_PARTY_DIR}/include)
link_directories(${CORE_3RD_PARTY_DIR}/lib ${PROJECT_DIR}/../bin)

add_executable(coreTests Tests.cpp)
set(TO_LINK_LIBS gtest Core pthread)
target_link_libraries(coreTests ${TO_LINK_LIBS})
