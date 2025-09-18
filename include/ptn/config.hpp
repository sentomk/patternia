// clang-format off
#ifndef PTN_ENABLE_VALUE_PATTERN
  #define PTN_ENABLE_VALUE_PATTERN 1
#endif

#ifndef PTN_ENABLE_RELATIONAL_PATTERN
  #define PTN_ENABLE_RELATIONAL_PATTERN 1
#endif

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
  #define PTN_USE_CONCEPTS 1
#else
  #define PTN_USE_CONCEPTS 0
#endif

