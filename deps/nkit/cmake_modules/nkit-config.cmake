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

## Find NKit

## input : NKIT_ROOT, YAJL_ROOT, EXPAT_ROOT
## output: NKIT_VERSION_{MAJOR,MINOR,MICRO,BUILD}, NKIT_VERSION,
##         NKIT_INCLUDE_DIR - nkit include dir
##         NKIT_LIBRARY - nkit library
##         NKIT_CMAKE_MODULE_PATH - path to custom nkit modules for cmake
##         YAJL_INCLUDE_DIR - yajl include dirs
##         YAJL_LIBRARIES - yajl libraries
##         YAJL_VERSION_{MAJOR,MINOR,MICRO}, YAJL_VERSION - yajl versions
##         NKIT_ALL_INCLUDE_DIRS - nkit & others include dirs
##         NKIT_ALL_LIBRARIES - nkit & others libraries

## TODO
## 1) Handling all find_package args(for detail see cmake --help-command find_package)
## 2) Hand pkgconfig
##

macro(nkit_version__)
  file(READ ${NKIT_INCLUDE_DIR}/nkit/version.h cont__)
  string(REGEX REPLACE
         "^(.*)define NKIT_VERSION_MAJOR \"([0-9]+).*$" "\\2"
         NKIT_VERSION_MAJOR "${cont__}")
  string(REGEX REPLACE
         "^(.*)define NKIT_VERSION_MINOR \"([0-9]+).*$" "\\2"
         NKIT_VERSION_MINOR "${cont__}")
  string(REGEX REPLACE
         "^(.*)define NKIT_VERSION_MICRO \"([0-9]+).*$" "\\2"
         NKIT_VERSION_MICRO "${cont__}")
  string(REGEX REPLACE
    "^(.*)define NKIT_VERSION_BUILD \"([0-9a-zA-Z]+).*$" "\\2"
         NKIT_VERSION_BUILD "${cont__}")
  if ((NKit_FIND_VERSION_MAJOR LESS NKIT_VERSION_MAJOR)
      AND (NKit_FIND_VERSION_MINOR LESS NKIT_VERSION_MINOR)
      AND (NKit_FIND_VERSION_PATCH LESS NKIT_VERSION_MICRO))
    message(SEND_ERROR
      "nkit have wrong version ${NKIT_VERSION} at I${NKIT_INCLUDE_DIR},L${NKIT_LIBRARIES}")
  endif()
  set(NKIT_VERSION
    "${NKIT_VERSION_MAJOR}.${NKIT_VERSION_MINOR}.${NKIT_VERSION_MICRO}.${NKIT_VERSION_BUILD}"
    CACHE INTERNAL "nkit version full")
endmacro(nkit_version__)


## Start here
if ((NKit_FIND_VERSION_MAJOR LESS 0) OR (NKit_FIND_VERSION_MAJOR EQUAL 0)
   AND (NKit_FIND_VERSION_MINOR LESS 0) OR (NKit_FIND_VERSION_MINOR EQUAL 0)
   AND (NKit_FIND_VERSION_PATCH LESS 30) OR (NKit_FIND_VERSION_PATCH EQUAL 0))
   message(SEND_ERROR
           "This module did not optimize for this version greater than 0.0.30.X")
endif()

message (STATUS "NKIT_ROOT: " ${NKIT_ROOT})
message (STATUS "YAJL_ROOT: " ${YAJL_ROOT})
message (STATUS "EXPAT_ROOT: " ${EXPAT_ROOT})

if (DEFINED NKIT_ROOT)
  set(NKit_INCLUDE_DIRS__  ${NKIT_ROOT}/include)
  set(NKit_LIB_DIRS__      ${NKIT_ROOT}/lib)
  set(NKit_FIND_OPTS__     NO_CMAKE NO_CMAKE_SYSTEM_PATH)
else()
  set(NKit_INCLUDE_DIRS__  /usr/local/include /usr/include)
  set(NKit_LIB_DIRS__      /usr/local/lib /usr/lib)
endif()

find_path(NKIT_INCLUDE_DIR
          NAMES nkit/
          HINTS ${NKit_INCLUDE_DIRS__}
          ${NKit_FIND_OPTS__})
if (${NKIT_INCLUDE_DIR} STREQUAL "NKIT_INCLUDE_DIR-NOTFOUND")
  message(SEND_ERROR "Could not find nkit include directories")
endif()

set(NKIT_LIB_NAME  libnkit.a)

find_path(NKIT_LIBRARY_DIR
          NAMES ${NKIT_LIB_NAME}
          HINTS ${NKit_LIB_DIRS__}
          ${NKit_FIND_OPTS__})
if (${NKIT_LIBRARY_DIR} STREQUAL "NKIT_LIBRARY_DIR-NOTFOUND")
  message(SEND_ERROR "Could not find nkit lib directory")
endif()

find_library(NKIT_LIBRARY NAMES ${NKIT_LIB_NAME} HINTS ${NKIT_LIBRARY_DIR})
if (${NKIT_LIBRARY} STREQUAL "NKIR_LIBRARY-NOTFOUND")
  message(SEND_ERROR "Could not find '${NKIT_LIB_NAME}' ('${NKIT_LIBRARY_DIR}')")
else()
  set(NKIT_LIBRARIES ${NKIT_LIBRARY})
endif()

nkit_version__()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NKit
      DEFAULT_MSG NKIT_INCLUDE_DIR NKIT_LIBRARIES NKIT_VERSION)
mark_as_advanced(NKIT_INCLUDE_DIR NKIT_LIBRARIES NKIT_VERSION)

# yajl
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${NKIT_LIBRARY_DIR}/nkit)
find_package(Yajl 2.0.4 REQUIRED)

# expat
if (DEFINED EXPAT_ROOT)
    if (WIN32 OR WIN64)
        set(EXPAT_INCLUDE_DIRS "${EXPAT_ROOT}/include/expat")
        set(EXPAT_LIBRARIES "${EXPAT_ROOT}/lib/libexpat.lib")
    else()
        set(EXPAT_INCLUDE_DIRS "${EXPAT_ROOT}/include")
        set(EXPAT_LIBRARIES "${EXPAT_ROOT}/lib/libexpat.a")
    endif()
endif()

# nkit & others include dirs & libraries
set(NKIT_ALL_INCLUDE_DIRS
    ${NKIT_INCLUDE_DIR}
    ${YAJL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
    )
set(NKIT_ALL_LIBRARIES
    ${NKIT_LIBRARY}
    ${YAJL_LIBRARIES}
    ${EXPAT_LIBRARIES}
    )
