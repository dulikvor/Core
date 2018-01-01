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
    find_package(SpdLog)
    include(installThirdParty)
endif()


if(NOT CORE_NO_GLOBAL_HANDLING)
    set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
endif()
include_directories(${PROJECT_3RD_LOC}/include ${PROJECT_DIR} ${PROJECT_DIR}/Third_Party/include)

#Compile staticly
if(SPDLOG_SUPPORT)
    set(SPDLOG_SRC src/SpdLogLogger.cpp src/SpdLogTraceListener.cpp)
else()
    set(SPDLOG_SRC)
endif()
add_library(Core STATIC src/AsyncExecutor.cpp src/AsyncTask.cpp src/AutoResetEvent.cpp src/ChildProcess.cpp src/CommandLine.cpp src/Directory.cpp src/Environment.cpp src/Logger.cpp src/Pipe.cpp src/Process.cpp src/TcpSocket.cpp src/Thread.cpp src/TimedAsyncExecutor.cpp ${SPDLOG_SRC})
