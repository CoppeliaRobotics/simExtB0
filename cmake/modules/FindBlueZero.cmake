cmake_minimum_required(VERSION 2.8.12)
include(CMakeParseArguments)

set(BLUEZERO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bluezero" CACHE STRING "BlueZero directory")
file(TO_CMAKE_PATH "${BLUEZERO_DIR}" BLUEZERO_DIR)

if(NOT EXISTS "${BLUEZERO_DIR}")
    message(FATAL_ERROR "BLUEZERO_DIR does not contain a valid directory")
else()
    set(BLUEZERO_BUILD_DIR "${BLUEZERO_DIR}/build" CACHE STRING "BlueZero build directory")
    file(TO_CMAKE_PATH "${BLUEZERO_BUILD_DIR}" BLUEZERO_BUILD_DIR)

    if(NOT EXISTS "${BLUEZERO_BUILD_DIR}")
        message(FATAL_ERROR "BLUEZERO_BUILD_DIR does not contain a valid directory")
    endif()
endif()

set(BLUEZERO_INCLUDE_SEARCH_PATHS
    /usr/include
    /usr/local/include
    ${BLUEZERO_DIR}/include
    ${BLUEZERO_BUILD_DIR}/include
)
find_path(BLUEZERO_INCLUDE_DIR_1 b0/b0.h HINTS ${BLUEZERO_INCLUDE_SEARCH_PATHS})
find_path(BLUEZERO_INCLUDE_DIR_2 b0/config.h HINTS ${BLUEZERO_INCLUDE_SEARCH_PATHS})

find_library(BLUEZERO_LIBRARY NAMES b0 HINTS ${BLUEZERO_BUILD_DIR})

find_package(Boost 1.54 REQUIRED COMPONENTS thread system regex timer filesystem serialization)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set BLUEZERO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(BlueZero  DEFAULT_MSG
    BLUEZERO_LIBRARY BLUEZERO_INCLUDE_DIR_1 BLUEZERO_INCLUDE_DIR_2)

set(BLUEZERO_LIBRARIES 
    ${Boost_LIBRARIES}
    ${BLUEZERO_LIBRARY}
)
unset(BLUEZERO_LIBRARY CACHE)
set(BLUEZERO_INCLUDE_DIRS
    ${Boost_INCLUDE_DIRS}
    ${BLUEZERO_INCLUDE_DIR_1}
    ${BLUEZERO_INCLUDE_DIR_2}
)
unset(BLUEZERO_INCLUDE_DIR_1 CACHE)
unset(BLUEZERO_INCLUDE_DIR_2 CACHE)

mark_as_advanced(BLUEZERO_LIBRARIES BLUEZERO_INCLUDE_DIRS)
