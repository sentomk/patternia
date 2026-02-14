cmake_minimum_required(VERSION 3.14...3.28)

# ========== Policies and compatibility ==========
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# ========== Project ==========
project(patternia
  VERSION 0.7.2
  DESCRIPTION "Header-only pattern matching library for modern C++"
  LANGUAGES CXX
)

# ========== Global language standard ==========
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

# ========== Utilities ==========
macro(PTN_SET_IF_UNDEFINED VAR VALUE)
  if(NOT DEFINED ${VAR})
    set(${VAR} ${VALUE})
  endif()
endmacro()

# Determine whether this CMakeLists is the top-level entry.
function(PTN_IS_MASTER_PROJECT OUT_VAR)
  if(NOT DEFINED PROJECT_NAME)
    # No project() above us
    set(${OUT_VAR} ON PARENT_SCOPE)
  else()
    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
      set(${OUT_VAR} ON PARENT_SCOPE)
    else()
      set(${OUT_VAR} OFF PARENT_SCOPE)
    endif()
  endif()
endfunction()

# Generate a tiny dummy .cc to force a real library for IDE indexing.
function(PTN_GENERATE_DUMMY_SOURCE OUT_ABS_PATH)
  set(_path "${CMAKE_CURRENT_SOURCE_DIR}/cmake/patternia_dummy.cc")

  if(NOT EXISTS "${_path}")
    file(WRITE "${_path}" "// IDE-only dummy TU\n#include \"ptn/patternia.hpp\"\n")
  endif()

  set(${OUT_ABS_PATH} "${_path}" PARENT_SCOPE)
endfunction()

function(PTN_JOIN_HEADERS OUT_VAR)
  set(_accum)

  foreach(_h IN LISTS ARGN)
    list(APPEND _accum "${_h}")
  endforeach()

  set(${OUT_VAR} "${_accum}" PARENT_SCOPE)
endfunction()

# ========== Master project detection and defaulted options ==========
PTN_IS_MASTER_PROJECT(_PTN_MASTER_PROJECT)
PTN_SET_IF_UNDEFINED(PTN_MASTER_PROJECT ${_PTN_MASTER_PROJECT})

# Options
option(PTN_INSTALL "Generate install/export targets" ${PTN_MASTER_PROJECT})
option(PTN_BUILD_TESTS "Build unit tests" ${PTN_MASTER_PROJECT})
option(PTN_BUILD_BENCHMARKS "Build benchmarks" ${PTN_MASTER_PROJECT})
option(PTN_DEV_INDEX "Create local indexable target for IDEs (not installed)" ON)
option(PTN_SKIP_COMPILER_CHECK "Skip compiler version check" OFF)

# ========== Public headers ==========
PTN_JOIN_HEADERS(PTN_HEADERS
  include/ptn/patternia.hpp

  # Core Layer
  include/ptn/core/fwd.h
  include/ptn/core/common/common_traits.hpp
  include/ptn/core/common/diagnostics.hpp
  include/ptn/core/common/eval.hpp
  include/ptn/core/common/optimize.hpp
  include/ptn/core/engine/builder.hpp
  include/ptn/core/engine/match.hpp
  include/ptn/core/engine/detail/builder_impl.hpp
  include/ptn/core/engine/detail/match_impl.hpp
  include/ptn/core/dsl/case_expr.hpp
  include/ptn/core/dsl/ops.hpp
  include/ptn/core/dsl/detail/case_expr_impl.hpp

  # Meta Utils
  include/ptn/meta/meta.hpp
  include/ptn/meta/base/traits.hpp
  include/ptn/meta/dsa/algorithms.hpp
  include/ptn/meta/dsa/type_list.hpp
  include/ptn/meta/query/index.hpp
  include/ptn/meta/query/template_info.hpp

  # Pattern Base
  include/ptn/pattern/base/fwd.h
  include/ptn/pattern/base/binding_base.hpp
  include/ptn/pattern/base/binding_traits.hpp
  include/ptn/pattern/base/pattern_base.hpp
  include/ptn/pattern/base/pattern_kind.hpp
  include/ptn/pattern/base/pattern_traits.hpp

  # Pattern Module Headers
  include/ptn/pattern/lit.hpp
  include/ptn/pattern/bind.hpp
  include/ptn/pattern/type.hpp
  include/ptn/pattern/wildcard.hpp
  include/ptn/pattern/structural.hpp
  include/ptn/pattern/modifiers/guard.hpp
  include/ptn/pattern/modifiers/fwd.h
)

# ========== Public interface target ==========
add_library(patternia INTERFACE)
add_library(patternia::patternia ALIAS patternia)

target_compile_features(patternia INTERFACE cxx_std_17)
target_include_directories(patternia
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# If available, attach file sets for better IDE awareness.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.23)
  target_sources(patternia INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES ${PTN_HEADERS}
  )
endif()

# ========== Local-only index target for IDEs ==========
if(PTN_MASTER_PROJECT AND PTN_DEV_INDEX)
  PTN_GENERATE_DUMMY_SOURCE(_PTN_DUMMY_TU)

  add_library(ptn.index STATIC
    ${PTN_HEADERS}
    ${_PTN_DUMMY_TU}
  )

  target_compile_features(ptn.index PUBLIC cxx_std_17)
  target_include_directories(ptn.index
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  # Inherit public interface properties from the INTERFACE target
  target_link_libraries(ptn.index PUBLIC patternia)

  # Not installed, not exported by design.
endif()

# ========== Optional: compiler version check ==========
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT PTN_SKIP_COMPILER_CHECK)
  # Provide your own cmake/CheckCompilerVersion.cmake for hard gating if needed.
  include(CheckCompilerVersion OPTIONAL RESULT_VARIABLE _PTN_HAS_COMPILER_CHECK)

  if(NOT _PTN_HAS_COMPILER_CHECK)
    message(STATUS "[patternia] Skipping compiler check (module not found).")
  endif()
endif()

# # ========== Tests (optional, master default ON) ==========
# if(PTN_BUILD_TESTS)
# include(FetchContent)

# # Configure GoogleTest (lightweight settings)
# set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
# set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
# FetchContent_Declare(
# googletest
# GIT_REPOSITORY https://github.com/google/googletest.git
# GIT_TAG v1.17.0
# GIT_SHALLOW TRUE
# )
# FetchContent_MakeAvailable(googletest)

# enable_testing()
# add_executable(ptn_tests
# tests/tests_literal_pattern.cpp
# tests/tests_relational_pattern.cpp
# tests/tests_predicate_pattern.cpp
# tests/tests_type_pattern.cpp
# )
# target_link_libraries(ptn_tests PRIVATE
# patternia
# GTest::gtest
# GTest::gtest_main
# )
# include(GoogleTest)
# gtest_discover_tests(ptn_tests)
# endif()

# # ========== Benchmarks (optional, master default ON) ==========
# if(PTN_BUILD_BENCHMARKS)
# include(FetchContent)
# find_package(Threads REQUIRED)
# set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
# set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

# FetchContent_Declare(
# benchmark
# GIT_REPOSITORY https://github.com/google/benchmark.git
# GIT_TAG v1.9.4
# )
# FetchContent_MakeAvailable(benchmark)

# add_executable(bench_patternia
# benchmarks/bench_main.cpp
# benchmarks/bench_patternia.cpp
# )
# target_link_libraries(bench_patternia PRIVATE
# patternia
# benchmark::benchmark
# Threads::Threads
# )
# target_compile_options(bench_patternia PRIVATE -O3 -march=native)
# endif()
add_executable(sample_fibo
  samples/fibo.cpp
)
target_link_libraries(sample_fibo PRIVATE patternia)

add_executable(sample_packet
  samples/handle_packet.cpp
)
target_link_libraries(sample_packet PRIVATE patternia)

add_executable(sample_test
  samples/test.cpp
)
target_link_libraries(sample_test PRIVATE patternia)

add_executable(sample_variant 
  samples/variant_type_is.cpp
)
target_link_libraries(sample_variant PRIVATE patternia)

# add_executable(json_parser
#   samples/json_dispatch/parse_json_pred.hpp
#   samples/json_dispatch/parse_json_handler.hpp
#   samples/json_dispatch/parse_json.cpp
#   samples/json_dispatch/main.cpp
# )
# target_link_libraries(json_parser PRIVATE patternia)
# find_package(nlohmann_json CONFIG REQUIRED)
# target_link_libraries(json_parser PRIVATE nlohmann_json::nlohmann_json)


# ========== Install / Export (only the INTERFACE target is exported) ==========
if(PTN_INSTALL)
  include(CMakePackageConfigHelpers)

  # Install the INTERFACE target and its headers.
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.23)
    install(TARGETS patternia
      EXPORT patterniaTargets
      FILE_SET HEADERS
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  else()
    # Fallback for older CMake: install the include tree explicitly.
    install(TARGETS patternia
      EXPORT patterniaTargets
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  endif()

  # Package config files.
  set(PTN_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/patternia)
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/patterniaConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
  )
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/patterniaConfig.cmake"
    INSTALL_DESTINATION "${PTN_CMAKE_DIR}"
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/patterniaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/patterniaConfigVersion.cmake"
    DESTINATION ${PTN_CMAKE_DIR}
  )

  install(EXPORT patterniaTargets
    FILE patterniaTargets.cmake
    NAMESPACE patternia::
    DESTINATION ${PTN_CMAKE_DIR}
  )

  # Export to the build tree for local find_package().
  export(
    EXPORT patterniaTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/patterniaTargets.cmake"
  )
endif()
