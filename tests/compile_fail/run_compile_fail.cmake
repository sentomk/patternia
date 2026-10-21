if(NOT DEFINED CXX_COMPILER)
  message(FATAL_ERROR "CXX_COMPILER is required")
endif()

if(NOT DEFINED SOURCE_FILE)
  message(FATAL_ERROR "SOURCE_FILE is required")
endif()

if(NOT DEFINED INCLUDE_DIR)
  message(FATAL_ERROR "INCLUDE_DIR is required")
endif()

if(NOT DEFINED OBJECT_FILE)
  message(FATAL_ERROR "OBJECT_FILE is required")
endif()

if(NOT DEFINED CXX_STANDARD)
  set(CXX_STANDARD 17)
endif()

get_filename_component(_obj_dir "${OBJECT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${_obj_dir}")

set(_is_msvc_style FALSE)
if(DEFINED CXX_COMPILER_ID AND CXX_COMPILER_ID STREQUAL "MSVC")
  set(_is_msvc_style TRUE)
elseif(DEFINED CXX_SIMULATE_ID AND CXX_SIMULATE_ID STREQUAL "MSVC")
  set(_is_msvc_style TRUE)
elseif(CXX_COMPILER MATCHES "([/\\\\]|^)(cl|clang-cl)(\\.exe)?$")
  set(_is_msvc_style TRUE)
endif()

if(_is_msvc_style)
  if(CXX_STANDARD GREATER_EQUAL 23)
    set(_std_flag "/std:c++latest")
  else()
    set(_std_flag "/std:c++${CXX_STANDARD}")
  endif()

  set(_compile_args
    "/nologo"
    "${_std_flag}"
    "/I${INCLUDE_DIR}"
    "/c" "${SOURCE_FILE}"
    "/Fo${OBJECT_FILE}"
  )
else()
  set(_compile_args
    "-std=c++${CXX_STANDARD}"
    "-I${INCLUDE_DIR}"
    "-c" "${SOURCE_FILE}"
    "-o" "${OBJECT_FILE}"
  )
endif()

execute_process(
  COMMAND "${CXX_COMPILER}" ${_compile_args}
  RESULT_VARIABLE _rv
  OUTPUT_VARIABLE _out
  ERROR_VARIABLE _err
)

if(_rv EQUAL 0)
  message(FATAL_ERROR
    "Expected compile failure but compilation succeeded: ${SOURCE_FILE}")
endif()

# Cases may pin a library diagnostic and reject cascading errors.
# Use literal substrings so punctuation in static_assert messages
# does not need regular-expression escaping in the source files.
file(STRINGS "${SOURCE_FILE}" _expectations
  REGEX "^// (EXPECT_ERROR|FORBID_ERROR): ")
foreach(_expectation IN LISTS _expectations)
  string(REGEX REPLACE "^// (EXPECT_ERROR|FORBID_ERROR): " ""
    _diagnostic "${_expectation}")
  string(FIND "${_out}${_err}" "${_diagnostic}" _position)
  if(_expectation MATCHES "^// EXPECT_ERROR:" AND _position EQUAL -1)
    message(FATAL_ERROR
      "Missing expected diagnostic: ${_diagnostic}\n${_out}${_err}")
  elseif(_expectation MATCHES "^// FORBID_ERROR:" AND NOT _position EQUAL -1)
    message(FATAL_ERROR
      "Unexpected cascading diagnostic: ${_diagnostic}\n${_out}${_err}")
  endif()
endforeach()

message(STATUS "Compile-fail test passed for ${SOURCE_FILE}")
