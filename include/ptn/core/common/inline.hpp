#pragma once

// Internal inlining control for the evaluation path.
//
// A match is only as cheap as hand-written code when the whole path
// from `match(x) | on(...)` down to the selected handler is inlined
// into the caller. Only then can the compiler see that the case pack
// is a set of constants and keep it out of memory. If any link of
// that chain stays out of line, every call first writes all cases to
// the stack and then reads one back, which costs time proportional
// to the number of cases.
//
// Compilers make this decision per function from size estimates, so
// an unrelated change (a new static check, one more case) can flip
// it silently. Functions on the dispatch path therefore request
// inlining explicitly instead of relying on the heuristic.
//
// The macro replaces the `inline` keyword. It is an implementation
// detail and not part of the public API.

#if defined(_MSC_VER)
#define PTN_DETAIL_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define PTN_DETAIL_ALWAYS_INLINE                                    \
  inline __attribute__((always_inline))
#else
#define PTN_DETAIL_ALWAYS_INLINE inline
#endif

// The same request for a lambda, written after its parameter list:
// `[&](auto &&x) PTN_DETAIL_LAMBDA_ALWAYS_INLINE -> T { ... }`.
// Continuations passed between the owned evaluation functions are
// lambdas, and GCC inlines a callee early only once it has processed
// it, which depends on the order of functions in the translation
// unit. MSVC has no portable spelling, so it relies on the
// heuristic.
#if defined(__GNUC__) || defined(__clang__)
#define PTN_DETAIL_LAMBDA_ALWAYS_INLINE                             \
  __attribute__((always_inline))
#else
#define PTN_DETAIL_LAMBDA_ALWAYS_INLINE
#endif
