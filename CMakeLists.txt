cmake_minimum_required(VERSION 2.8.11)
project(simExtBlueZero)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

if(WIN32)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
    add_definitions(-DBOOST_ALL_NO_LIB)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DBOOST_USE_WINDOWS_H)
    add_definitions(-DNOMINMAX)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0.0)
    add_compile_options(-fpermissive)
endif()

find_package(Boost REQUIRED COMPONENTS thread system filesystem)

if(NOT LIBPLUGIN_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
    else()
        set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${LIBPLUGIN_DIR}/cmake)
find_package(CoppeliaSim 4.1.0.0 REQUIRED)
find_package(BlueZero REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${BLUEZERO_INCLUDE_DIRS})

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simExtBlueZero.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

coppeliasim_add_plugin(simExtBlueZero SOURCES plugin.cpp)
target_link_libraries(simExtBlueZero ${BLUEZERO_LIBRARIES})

if(WIN32)
    find_package(LZ4)
    if(LZ4_FOUND)
        target_link_libraries(simExtBlueZero lz4::lz4)
    endif()
endif()
