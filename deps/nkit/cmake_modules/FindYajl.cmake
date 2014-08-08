#   Copyright 2014 Vasiliy Soshnikov (dedok.mad@gmail.com)
#                  Boris T. Darchiev (boris.darchiev@gmail.com)
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

# Find yajl version 2.X.X
#
# TODO Add search in pgk config

if (Yajl_FIND_VERSION_EXACT)
    if (${Yajl_FIND_VERSION_MAJOR} LESS 2)
        message(SEND_ERROR
          "This module did not optimize for this version of yajl")
    endif()
endif()

if (DEFINED YAJL_ROOT)
    set(_INCLUDE_DIRS ${YAJL_ROOT}/include)
    set(_LIB_DIRS ${YAJL_ROOT}/lib)
    set(_FIND_OPTS NO_CMAKE NO_CMAKE_SYSTEM_PATH)
else()
    set(_INCLUDE_DIRS /usr/local/include /usr/include)
    set(_LIB_DIRS /usr/local/lib /usr/lib)
endif()

find_path(YAJL NAMES yajl/ HINTS ${_INCLUDE_DIRS} ${_FIND_OPTS})

if (NOT ${YAJL} STREQUAL "YAJL-NOTFOUND")
    set(YAJL_INCLUDE_DIR ${YAJL} CACHE INTERNAL "Yajl include directory")

    file(READ "${YAJL}/yajl/yajl_version.h" content_)
    string(REGEX REPLACE
           "^(.*)define YAJL_MAJOR ([0-9]).*$" "\\2"
           YAJL_MAJOR "${content_}")
    string(REGEX REPLACE
          "^(.*)define YAJL_MINOR ([0-9]).*$" "\\2"
          YAJL_MINOR "${content_}")
    string(REGEX REPLACE
          "^(.*)define YAJL_MICRO ([0-9]).*$" "\\2"
          YAJL_MICRO "${content_}")

    set(YAJL_VERSION "${YAJL_MAJOR}.${YAJL_MINOR}.${YAJL_MICRO}"
        CACHE INTERNAL "Yajl version")
    
    if (WIN32 OR WIN64)
      set(YAJL_USE_DYN_LIBS 1)
    endif()

    if (DEFINED YAJL_USE_DYN_LIBS)
        find_library(YAJL_LIB_DYN
                     NAMES libyajl yajl
                     HINTS ${_LIB_DIRS}
                     ${_FIND_OPTS})
        if (NOT ${YAJL_LIB_DYN} STREQUAL "YAJL_LIB_DYN-NOTFOUND")
          set(YAJL_LIBRARIES ${YAJL_LIB_DYN} CACHE INTERNAL "Yajl libraries")
        endif()
    else()
        find_library(YAJL_LIB NAMES libyajl_s yajl_s HINTS ${_LIB_DIRS})
        if (NOT ${YAJL_LIB} STREQUAL "YAJL_LIB_DYN-NOTFOUND")
          set(YAJL_LIBRARIES ${YAJL_LIB} CACHE INTERNAL "Yajl libraries")
        endif()
    endif()

    if (NOT YAJL_NOT_FOUND)
      set(YAJL_FOUND TRUE CACHE INTERNAL "Yajl found")
        set(HAVE_YAJL 1 CACHE INTERNAL "FindYajl.cmake -> have yajl")
    else()
      set(YAJL_FOUND FALSE CACHE INTERNAL "Yajl found")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Yajl
      REQUIRED_VARS YAJL_INCLUDE_DIR YAJL_LIBRARIES
      VERSION_VAR YAJL_VERSION)
mark_as_advanced(YAJL_INCLUDE_DIR YAJL_LIBRARIES YAJL_VERSION)
