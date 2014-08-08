# ADD_PRECOMPILED_HEADER( targetName HEADERS _inputs )
MACRO(ADD_PRECOMPILED_HEADER _targetName _inputs )
  FOREACH (_current_FILE ${ARGN})

    GET_FILENAME_COMPONENT(_name ${_current_FILE} NAME)
    SET(_source "${_current_FILE}")
    SET(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${_name}.gch")
    file(MAKE_DIRECTORY ${_outdir})
    SET(_output "${_outdir}/${CMAKE_BUILD_TYPE}.c++")
    STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
    SET(_compiler_FLAGS ${${_flags_var_name}})

    GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
    FOREACH(item ${_directory_flags})
      LIST(APPEND _compiler_FLAGS "-I${item}")
    ENDFOREACH(item)

    GET_DIRECTORY_PROPERTY(_directory_flags DEFINITIONS)
    LIST(APPEND _compiler_FLAGS ${_directory_flags})

    GET_TARGET_PROPERTY(_oldFlags ${_targetName} COMPILE_FLAGS)
    LIST(APPEND _compiler_FLAGS ${_oldFlags})

    SEPARATE_ARGUMENTS(_compiler_FLAGS)

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_output}
      COMMAND ${CMAKE_CXX_COMPILER}
      ${_compiler_FLAGS}
      -x c++-header
      -o ${_output} ${_source}
      DEPENDS ${_source} )
    ADD_CUSTOM_TARGET(${_targetName}_gch DEPENDS ${_output})
    ADD_DEPENDENCIES(${_targetName} ${_targetName}_gch)
    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES
      COMPILE_FLAGS "${_oldFlags} -include ${_name} -Winvalid-pch"
      )
  ENDFOREACH (_current_FILE)
ENDMACRO(ADD_PRECOMPILED_HEADER)

# Lets say you have a variable ${MySources} with all your sourcefiles, 
# the code you would want to use would be simply be:
# ADD_MSVC_PRECOMPILED_HEADER("precompiled.h" "precompiled.cpp" ${MySources})
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource)
  IF(MSVC)
    foreach( src_file ${ARGN} )
      if(${src_file} STREQUAL ${PrecompiledSource})
        set_source_files_properties(
            ${PrecompiledSource}
            PROPERTIES
            COMPILE_FLAGS "/Yc${PrecompiledHeader}"
            )
      else()
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/Yu${PrecompiledHeader}"
            )
      endif()
    endforeach( src_file ${ARGN} )
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)
