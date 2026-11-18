# Installs the build tree into a scratch prefix, checks that every
# header under include/ was installed, then builds and runs a consumer
# that finds the package with find_package(patternia).
#
# Inputs: BUILD_DIR, SOURCE_DIR, WORK_DIR, CXX_COMPILER, GENERATOR,
# CONFIG (may be empty).

set(_prefix "${WORK_DIR}/prefix")
set(_consumer_build "${WORK_DIR}/consumer")
file(REMOVE_RECURSE "${WORK_DIR}")

set(_config_args "")
if(CONFIG)
  set(_config_args --config "${CONFIG}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${BUILD_DIR}"
          --prefix "${_prefix}" ${_config_args}
  RESULT_VARIABLE _rc OUTPUT_QUIET)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR "cmake --install failed: ${_rc}")
endif()

file(GLOB_RECURSE _source_headers RELATIVE "${SOURCE_DIR}/include"
  "${SOURCE_DIR}/include/ptn/*")
set(_missing "")
foreach(_h IN LISTS _source_headers)
  if(NOT EXISTS "${_prefix}/include/${_h}")
    list(APPEND _missing "${_h}")
  endif()
endforeach()
if(_missing)
  string(REPLACE ";" "\n  " _missing "${_missing}")
  message(FATAL_ERROR "Headers not installed:\n  ${_missing}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}"
          -S "${SOURCE_DIR}/tests/install/consumer"
          -B "${_consumer_build}"
          -G "${GENERATOR}"
          "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}"
          "-DCMAKE_PREFIX_PATH=${_prefix}"
          "-DCMAKE_BUILD_TYPE=Release"
  RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR "Configuring the consumer failed: ${_rc}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${_consumer_build}" --config Release
  RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR "Building the consumer failed: ${_rc}")
endif()

find_program(_exe ptn_install_consumer
  PATHS "${_consumer_build}" "${_consumer_build}/Release"
  NO_DEFAULT_PATH)
if(NOT _exe)
  message(FATAL_ERROR "Consumer executable not found")
endif()
execute_process(COMMAND "${_exe}" RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR "Consumer returned ${_rc}")
endif()
