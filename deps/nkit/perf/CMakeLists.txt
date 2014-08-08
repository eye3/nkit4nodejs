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

#if (WITH_PERF STREQUAL "1")
if (WITH_PERF)
  
  include_directories(BEFORE
      ## 3rd
      ${CMAKE_CURRENT_SOURCE_DIR}/
      ${CMAKE_CURRENT_SOURCE_DIR}/../3rd/sqlite3/
  )

  add_executable(perf_dynamic_table
      ${CMAKE_CURRENT_SOURCE_DIR}/perf_dynamic_table.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../3rd/sqlite3/sqlite3.c
  )

if (UNIX OR APPLE)
  target_link_libraries(perf_dynamic_table
      ${PROJECT_NAME}
      ${EXTRA_SYS_LIB}
      dl
  )
else()
  target_link_libraries(perf_dynamic_table
      ${PROJECT_NAME}
      ${EXTRA_SYS_LIB}
  )
endif(UNIX OR APPLE)

endif()
