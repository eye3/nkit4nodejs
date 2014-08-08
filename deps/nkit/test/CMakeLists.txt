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

### Test cases

if (EXPAT_FOUND)
  set(TEST_VX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_vx.cpp)
  set(EXTRA_SYS_LIB ${EXTRA_SYS_LIB} ${EXPAT_LIBRARIES})
else()
  set(TEST_VX_SOURCES "")
endif()


add_executable(dynamic
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dynamic.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dynamic_int.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dynamic_table.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dynamic_json.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_dynamic_datetime.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_tools.cpp
  ${TEST_VX_SOURCES}
)

target_link_libraries(dynamic ${PROJECT_NAME} ${EXTRA_SYS_LIB})

add_executable(logger ${CMAKE_CURRENT_SOURCE_DIR}/test_logger.cpp)
if (WIN32)
  set(EXTRA_SYS_LIB ${EXTRA_SYS_LIB} "ws2_32.lib")
endif(WIN32)

target_link_libraries(logger ${PROJECT_NAME} ${EXTRA_SYS_LIB})

### Auto tests
add_custom_target(compiling ALL DEPENDS dynamic logger VERBATIM)

add_test(compile-all
         ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target compiling)
set_tests_properties(compile-all PROPERTIES DEPENDS compiling)

add_test(dynamic-auto-test
    ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE}-build/test/dynamic)
add_test(logger-auto-test
    ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE}-build/test/logger)

