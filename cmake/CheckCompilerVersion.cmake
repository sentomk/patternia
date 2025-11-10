# cmake/CheckCompilerVersion.cmake
# Guard: allow skipping in special environments
if(DEFINED PTN_SKIP_COMPILER_CHECK AND PTN_SKIP_COMPILER_CHECK)
  message(STATUS "[patternia] Compiler version check is skipped (PTN_SKIP_COMPILER_CHECK=ON).")
  return()
endif()

# Helper: pretty print what we detected
function(_ptn_report_detected)
  message(STATUS "[patternia] C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

  if(DEFINED CMAKE_CXX_SIMULATE_ID)
    message(STATUS "[patternia] Simulate ID: ${CMAKE_CXX_SIMULATE_ID} ${CMAKE_CXX_SIMULATE_VERSION}")
  endif()

  if(MSVC)
    # MSVC_VERSION is an integer like 1930, 1938... -> treat as 19.30, 19.38
    if(DEFINED MSVC_VERSION)
      string(REGEX REPLACE "^([0-9]{2})([0-9]{2}).*$" "19.\\2" _ptn_msvc_str "${MSVC_VERSION}")
      message(STATUS "[patternia] MSVC toolset: ${MSVC_VERSION} (≈ ${_ptn_msvc_str})")
    endif()
  endif()
endfunction()

_ptn_report_detected()

# Minimums (configurable via cache)
set(PTN_MIN_GNU_VERSION "11" CACHE STRING "Minimum supported GCC version")
set(PTN_MIN_CLANG_VERSION "12" CACHE STRING "Minimum supported pure Clang version")
set(PTN_MIN_CLANG_CL_VERSION "18" CACHE STRING "Minimum supported clang-cl version")
set(PTN_MIN_APPLECLANG_VERSION "14" CACHE STRING "Minimum supported AppleClang version")
set(PTN_MIN_MSVC_VERSION_STR "19.30" CACHE STRING "Minimum supported MSVC (cl.exe) version")

# Helper to normalize MSVC_VERSION
function(_ptn_msvc_version_string out_var)
  if(NOT DEFINED MSVC_VERSION)
    set(${out_var} "" PARENT_SCOPE)
    return()
  endif()

  string(REGEX REPLACE "^([0-9]{2})([0-9]{2}).*$" "19.\\2" _ver "${MSVC_VERSION}")
  set(${out_var} "${_ver}" PARENT_SCOPE)
endfunction()

# ---- Detect clang-cl (Clang front-end with MSVC toolchain) ----
set(_ptn_is_clang_cl OFF)

if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(_ptn_is_clang_cl ON)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  set(_ptn_is_clang_cl ON)
endif()

# ---- Start checks ----
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS PTN_MIN_GNU_VERSION)
    message(FATAL_ERROR
      "[patternia] GCC ${CMAKE_CXX_COMPILER_VERSION} detected, but GCC >= ${PTN_MIN_GNU_VERSION} is required.\n"
      "Please upgrade your GCC toolchain.")
  endif()

elseif(_ptn_is_clang_cl)
  # clang-cl path: need both Clang >= 18 and MSVC >= 19.30
  # Clang version
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS PTN_MIN_CLANG_CL_VERSION)
    message(FATAL_ERROR
      "[patternia] clang-cl ${CMAKE_CXX_COMPILER_VERSION} detected, but Clang >= ${PTN_MIN_CLANG_CL_VERSION} is required for MSVC toolchain.\n"
      "Please upgrade LLVM/Clang.")
  endif()

  # MSVC toolset version
  _ptn_msvc_version_string(_ptn_msvc_str)

  if(_ptn_msvc_str STREQUAL "")
    # Fallback to the simulated version if available
    set(_ptn_msvc_str "${CMAKE_CXX_SIMULATE_VERSION}")
  endif()

  if(_ptn_msvc_str STREQUAL "")
    message(WARNING "[patternia] Unable to determine underlying MSVC toolset version; proceeding, but build may fail on old toolsets.")
  else()
    if(_ptn_msvc_str VERSION_LESS PTN_MIN_MSVC_VERSION_STR)
      message(FATAL_ERROR
        "[patternia] clang-cl uses MSVC toolset ${_ptn_msvc_str}, but MSVC >= ${PTN_MIN_MSVC_VERSION_STR} is required.\n"
        "Please install Visual Studio 2022 toolset 19.30+ (v143) or newer.")
    endif()
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Pure Clang (non-MSVC)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS PTN_MIN_CLANG_VERSION)
    message(FATAL_ERROR
      "[patternia] Clang ${CMAKE_CXX_COMPILER_VERSION} detected, but Clang >= ${PTN_MIN_CLANG_VERSION} is required.\n"
      "Please upgrade LLVM/Clang.")
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS PTN_MIN_APPLECLANG_VERSION)
    message(FATAL_ERROR
      "[patternia] AppleClang ${CMAKE_CXX_COMPILER_VERSION} detected, but AppleClang >= ${PTN_MIN_APPLECLANG_VERSION} is required.\n"
      "Please upgrade Xcode/Command Line Tools.")
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Native MSVC (cl.exe)
  _ptn_msvc_version_string(_ptn_msvc_str)

  if(_ptn_msvc_str STREQUAL "")
    message(WARNING "[patternia] Unable to determine MSVC toolset version; proceeding, but build may fail on old toolsets.")
  elseif(_ptn_msvc_str VERSION_LESS PTN_MIN_MSVC_VERSION_STR)
    message(FATAL_ERROR
      "[patternia] MSVC ${_ptn_msvc_str} detected, but MSVC >= ${PTN_MIN_MSVC_VERSION_STR} (VS2022) is required.\n"
      "Please upgrade to Visual Studio 2022 or newer.")
  endif()

else()
  message(FATAL_ERROR
    "[patternia] Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}. "
    "Supported: GNU, Clang, AppleClang, MSVC (and clang-cl).")
endif()

message(STATUS "[patternia] Compiler version check: PASSED")
