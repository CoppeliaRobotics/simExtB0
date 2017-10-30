cmake_minimum_required(VERSION 2.8.11)
project(v_repExtBlueZero)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(Boost REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
find_package(VREP REQUIRED)

include_directories(${VREP_INCLUDE})
include_directories(${CMAKE_BINARY_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${Boost_INCLUDE_DIRS})

vrep_generate_stubs(${CMAKE_BINARY_DIR}/generated XML_FILE ${CMAKE_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_SOURCE_DIR}/simExtBlueZero.lua)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)

add_library(v_repExtBlueZero
    SHARED
    plugin.cpp
    ${CMAKE_SOURCE_DIR}/external/v_repPlusPlus/Plugin.cpp
    ${CMAKE_BINARY_DIR}/generated/stubs.cpp
    ${VREP_EXPORTED_SOURCES}
)
target_link_libraries(v_repExtBlueZero ${Boost_LIBRARIES} ${VREP_LIBRARIES})

