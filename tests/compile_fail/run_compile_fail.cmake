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

execute_process(
  COMMAND "${CXX_COMPILER}" "-std=c++${CXX_STANDARD}" "-I${INCLUDE_DIR}" "-c" "${SOURCE_FILE}" "-o" "${OBJECT_FILE}"
  RESULT_VARIABLE _rv
  OUTPUT_VARIABLE _out
  ERROR_VARIABLE _err
)

if(_rv EQUAL 0)
  message(FATAL_ERROR
    "Expected compile failure but compilation succeeded: ${SOURCE_FILE}")
endif()

message(STATUS "Compile-fail test passed for ${SOURCE_FILE}")
