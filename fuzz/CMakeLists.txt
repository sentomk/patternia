# Fuzz targets (libFuzzer). Built only when PTN_BUILD_FUZZ=ON.
#
# Sanitizer set is platform-dependent: Windows supports fuzzer and
# address reliably; Linux additionally gets undefined.

if(WIN32)
  set(PTN_FUZZ_SANITIZERS "fuzzer,address")
else()
  set(PTN_FUZZ_SANITIZERS "fuzzer,address,undefined")
endif()

add_executable(ptn_fuzz_match fuzz_match.cpp)
target_link_libraries(ptn_fuzz_match PRIVATE patternia)
target_compile_features(ptn_fuzz_match PRIVATE cxx_std_17)
target_compile_options(ptn_fuzz_match PRIVATE
  -fsanitize=${PTN_FUZZ_SANITIZERS} -g -O1
)
target_link_options(ptn_fuzz_match PRIVATE
  -fsanitize=${PTN_FUZZ_SANITIZERS}
)

if(WIN32)
  # clang_rt.fuzzer ships as MT_StaticRelease. Pin the fuzz target
  # to the static release CRT (otherwise Debug trees produce
  # msvcrtd objects and /failifmismatch rejects the link), and
  # disable iterator debugging for the same reason.
  set_target_properties(ptn_fuzz_match PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded"
  )
  target_compile_options(ptn_fuzz_match PRIVATE
    -U_DEBUG -DNDEBUG -D_ITERATOR_DEBUG_LEVEL=0
  )
endif()
