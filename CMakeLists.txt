project(Core CXX)
cmake_minimum_required(VERSION 2.8)

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
    set(CMAKE_DEBUG_POSTFIX d)
endif()

if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(Red "${Esc}[31m")
    set(Green "${Esc}[32m")
endif()

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT CORE_3RD_PARTY_DIR)
    set(CORE_3RD_PARTY_DIR ${PROJECT_SOURCE_DIR}/Third_Party/)
endif()

option(CORE_SPDLOG_SUPPORT "SpdLog will be added as logger impel" OFF)
if(CORE_SPDLOG_SUPPORT)
    set(SPDLOG_SRC src/SpdLogLogger.cpp src/SpdLogTraceListener.cpp)
else()
    set(SPDLOG_SRC src/DefaultLogger.cpp src/DefaultLogger.h src/DefaultTraceListeners.h src/DefaultTraceListeners.cpp)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${CORE_3RD_PARTY_DIR})
find_package(SpdLog)

if(CORE_SPDLOG_SUPPORT AND CORE_PRE_COMPILE_STEP)
    include(installThirdParty)
endif()

#Compile staticly
if(CORE_COMPILE_STEP)
    if(WIN32)
        add_definitions(-DCORE_DLL)
    endif()
    include_directories(${CORE_3RD_PARTY_DIR}/include .)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
    add_library(Core SHARED src/AsyncExecutor.cpp src/AsyncTask.cpp src/AutoResetEvent.cpp src/ChildProcess.cpp src/CommandLine.cpp src/Directory.cpp src/Environment.cpp src/Logger.cpp src/Pipe.cpp src/Process.cpp src/TcpSocket.cpp src/TimedAsyncExecutor.cpp src/DefaultLogger.cpp src/DefaultTraceListeners.cpp ${SPDLOG_SRC})
    add_subdirectory(example)
endif()


if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-unused-variable -pedantic")
endif()
