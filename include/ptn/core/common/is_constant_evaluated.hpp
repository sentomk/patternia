#pragma once

// Portability shim for std::is_constant_evaluated().
// Provides PTN_DETAIL_IS_CONSTANT_EVALUATED() that works C++17→20.
//
// On C++20: delegates to std::is_constant_evaluated().
// On C++17 with compiler builtin: delegates to the builtin.
// Fallback: returns false (constexpr calls hit the eager path;
// constexpr usage of match|on with a stateless pack will hard-error
// at the function-local static instead of silently misbehaving).

#include <type_traits>

#if __cpp_lib_is_constant_evaluated >= 201811L
// C++20 standard library path
#define PTN_DETAIL_IS_CONSTANT_EVALUATED()                          \
  std::is_constant_evaluated()
#elif defined(__has_builtin)                                        \
    && __has_builtin(__builtin_is_constant_evaluated)
// Clang ≥9 / GCC ≥10 (via __has_builtin)
#define PTN_DETAIL_IS_CONSTANT_EVALUATED()                          \
  __builtin_is_constant_evaluated()
#elif defined(__GNUC__) && __GNUC__ >= 9 && !defined(__clang__)
// GCC 9.x (before __has_builtin support for this builtin)
#define PTN_DETAIL_IS_CONSTANT_EVALUATED()                          \
  __builtin_is_constant_evaluated()
#elif defined(_MSC_VER) && _MSC_VER >= 1925
// MSVC ≥16.0 / Visual Studio 2019
#define PTN_DETAIL_IS_CONSTANT_EVALUATED()                          \
  __builtin_is_constant_evaluated()
#else
// Fallback: no constant-evaluation detection available.
// match|on with stateless pack under constexpr will fail to compile
// (function-local static in constexpr context), which is strictly
// better than silently returning a wrong answer.
#define PTN_DETAIL_IS_CONSTANT_EVALUATED() false
#endif
