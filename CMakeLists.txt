project(Core CXX)
cmake_minimum_required(VERSION 2.8)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_COLOR_MAKEFILE ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Default build type 'Debug'")
    set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "" FORCE )
else()
    string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

if(NOT CORE_NO_3RD_PARTY)
    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    list(APPEND CMAKE_PREFIX_PATH ${EXTERNAL_PROJECT_DIR})
    find_package(SpdLog REQUIRED)
    include(installThirdParty)
endif()


if(NOT CORE_NO_GLOBAL_HANDLING)
    set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
    include_directories(. ${PROJECT_DIR}/Third_Party/include)
endif()
#Add all needed files
file(GLOB_RECURSE MYSources src/*.cpp)
message(STATUS "CXX files - src/${MYSources}")
#Compile staticly
add_library(Core STATIC ${MYSources})
