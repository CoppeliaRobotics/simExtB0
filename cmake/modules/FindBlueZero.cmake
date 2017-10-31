cmake_minimum_required(VERSION 2.8.12)
include(CMakeParseArguments)

if(NOT BLUEZERO_DIR)
    if(NOT DEFINED ENV{BLUEZERO_DIR})
        if(EXISTS "${CMAKE_SOURCE_DIR}/../bluezero")
            set(BLUEZERO_DIR "${CMAKE_SOURCE_DIR}/../bluezero")
        else()
            message(FATAL_ERROR "Cannot find BlueZero. Please set the BLUEZERO_DIR environment variable.")
        endif()
    else()
        set(BLUEZERO_DIR "$ENV{BLUEZERO_DIR}")
    endif()
endif()

file(TO_CMAKE_PATH "${BLUEZERO_DIR}" BLUEZERO_DIR)

if(NOT BLUEZERO_BUILD_DIR)
    if(NOT DEFINED ENV{BLUEZERO_BUILD_DIR})
        if(EXISTS "${BLUEZERO_DIR}/build")
            set(BLUEZERO_BUILD_DIR "${BLUEZERO_DIR}/build")
        else()
            message(FATAL_ERROR "Cannot find BlueZero build dir. Please set the BLUEZERO_BUILD_DIR environment variable.")
        endif()
    else()
        set(BLUEZERO_BUILD_DIR "$ENV{BLUEZERO_BUILD_DIR}")
    endif()
endif()

file(TO_CMAKE_PATH "${BLUEZERO_BUILD_DIR}" BLUEZERO_BUILD_DIR)

set(BLUEZERO_INCLUDE_SEARCH_PATHS
    /usr/include
    /usr/local/include
    ${BLUEZERO_DIR}/include
    ${BLUEZERO_BUILD_DIR}/include
)
find_path(BLUEZERO_INCLUDE_DIR_1 b0/b0.h HINTS ${BLUEZERO_INCLUDE_SEARCH_PATHS})
find_path(BLUEZERO_INCLUDE_DIR_2 b0/config.h HINTS ${BLUEZERO_INCLUDE_SEARCH_PATHS})
#if((BLUEZERO_INCLUDE_DIR_1 MATCHES NOTFOUND) OR (BLUEZERO_INCLUDE_DIR_2 MATCHES NOTFOUND))
#    set(BLUEZERO_INCLUDE_DIRS "BLUEZERO_INCLUDE_DIRS-NOTFOUND")
#else()
#    set(BLUEZERO_INCLUDE_DIRS ${BLUEZERO_INCLUDE_DIR_1} ${BLUEZERO_INCLUDE_DIR_2})
#endif()

find_library(BLUEZERO_LIBRARY NAMES b0 HINTS ${BLUEZERO_BUILD_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set BLUEZERO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(BlueZero  DEFAULT_MSG
    BLUEZERO_LIBRARY BLUEZERO_INCLUDE_DIR_1 BLUEZERO_INCLUDE_DIR_2)

set(BLUEZERO_LIBRARIES ${BLUEZERO_LIBRARY})
set(BLUEZERO_INCLUDE_DIRS ${BLUEZERO_INCLUDE_DIR_1} ${BLUEZERO_INCLUDE_DIR_2})

