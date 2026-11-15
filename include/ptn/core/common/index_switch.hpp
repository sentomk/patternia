#pragma once

// Dense index dispatch: the single place where Patternia turns
// "which arm" into control flow.
//
// `index_switch<N, Result, Arm>(index, args...)` runs
// `Arm::hit<I, Result>(args...)` for the arm `I == index` and
// `Arm::miss<Result>(args...)` when `index >= N`. It is a real
// `switch` over `0..N-1`, so the compiler lowers it the way it
// lowers a hand-written one: a lookup table when every arm yields a
// constant, a jump table otherwise.
//
// The arm is a type, not a callable object. Nothing is captured, so
// no state has to survive in memory between selecting an arm and
// running it, and every step can be forced inline. See inline.hpp
// for why that matters.

#include <cstddef>
#include <utility>

#include "ptn/core/common/inline.hpp"

#if defined(__GNUC__) || defined(__clang__)
#define PTN_DETAIL_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define PTN_DETAIL_UNLIKELY(x) (x)
#endif

namespace ptn::core::common::detail {

  // Largest index range handled by a switch. Wider ranges need
  // another strategy; callers check this before choosing
  // index_switch.
  constexpr std::size_t k_index_switch_max_arms = 512;

  template <std::size_t I,
            std::size_t N,
            typename Result,
            typename Arm,
            typename... Args>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  index_switch_arm(Args &&...args) {
    if constexpr (I < N) {
      return Arm::template hit<I, Result>(
          std::forward<Args>(args)...);
    }
    else {
      return Arm::template miss<Result>(std::forward<Args>(args)...);
    }
  }

  // Up to this many arms, compare the index in order instead of
  // switching on it. The caller has already rejected an index past
  // the last arm, so the last arm needs no comparison: two arms
  // become one test. GCC lowers a two-arm switch to branchless code
  // whose result feeds the caller's dependency chain; from three
  // arms on the switch wins.
  constexpr std::size_t k_index_switch_chain_max_arms = 2;

  template <std::size_t I,
            std::size_t N,
            typename Result,
            typename Arm,
            typename... Args>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  index_chain(std::size_t index, Args &&...args) {
    if constexpr (I + 1 == N) {
      return Arm::template hit<I, Result>(
          std::forward<Args>(args)...);
    }
    else {
      if (index == I) {
        return Arm::template hit<I, Result>(
            std::forward<Args>(args)...);
      }
      return index_chain<I + 1, N, Result, Arm>(
          index, std::forward<Args>(args)...);
    }
  }

#define PTN_DETAIL_INDEX_SWITCH_CASE(n)                             \
  case (n):                                                         \
    return index_switch_arm<(n), N, Result, Arm>(                   \
        std::forward<Args>(args)...)

#define PTN_DETAIL_INDEX_SWITCH_BLOCK_4(b)                          \
  PTN_DETAIL_INDEX_SWITCH_CASE((b) + 0);                            \
  PTN_DETAIL_INDEX_SWITCH_CASE((b) + 1);                            \
  PTN_DETAIL_INDEX_SWITCH_CASE((b) + 2);                            \
  PTN_DETAIL_INDEX_SWITCH_CASE((b) + 3)

#define PTN_DETAIL_INDEX_SWITCH_BLOCK_16(b)                         \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_4((b) + 0);                         \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_4((b) + 4);                         \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_4((b) + 8);                         \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_4((b) + 12)

#define PTN_DETAIL_INDEX_SWITCH_BLOCK_64(b)                         \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_16((b) + 0);                        \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_16((b) + 16);                       \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_16((b) + 32);                       \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_16((b) + 48)

#define PTN_DETAIL_INDEX_SWITCH_BLOCK_256(b)                        \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_64((b) + 0);                        \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_64((b) + 64);                       \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_64((b) + 128);                      \
  PTN_DETAIL_INDEX_SWITCH_BLOCK_64((b) + 192)

  // One function per size class, so a match with a handful of arms
  // does not instantiate hundreds of dead ones.
#define PTN_DETAIL_DEFINE_INDEX_SWITCH(name, blocks)                \
  template <std::size_t N,                                          \
            typename Result,                                        \
            typename Arm,                                           \
            typename... Args>                                       \
  PTN_DETAIL_ALWAYS_INLINE constexpr Result name(std::size_t index, \
                                                 Args &&...args) {  \
    switch (index) {                                                \
      blocks;                                                       \
    default:                                                        \
      return Arm::template miss<Result>(                            \
          std::forward<Args>(args)...);                             \
    }                                                               \
  }

  PTN_DETAIL_DEFINE_INDEX_SWITCH(index_switch_4,
                                 PTN_DETAIL_INDEX_SWITCH_BLOCK_4(0))
  PTN_DETAIL_DEFINE_INDEX_SWITCH(index_switch_16,
                                 PTN_DETAIL_INDEX_SWITCH_BLOCK_16(0))
  PTN_DETAIL_DEFINE_INDEX_SWITCH(index_switch_64,
                                 PTN_DETAIL_INDEX_SWITCH_BLOCK_64(0))
  PTN_DETAIL_DEFINE_INDEX_SWITCH(
      index_switch_256, PTN_DETAIL_INDEX_SWITCH_BLOCK_256(0))
  PTN_DETAIL_DEFINE_INDEX_SWITCH(
      index_switch_512, PTN_DETAIL_INDEX_SWITCH_BLOCK_256(0);
      PTN_DETAIL_INDEX_SWITCH_BLOCK_256(256))

#undef PTN_DETAIL_DEFINE_INDEX_SWITCH
#undef PTN_DETAIL_INDEX_SWITCH_BLOCK_256
#undef PTN_DETAIL_INDEX_SWITCH_BLOCK_64
#undef PTN_DETAIL_INDEX_SWITCH_BLOCK_16
#undef PTN_DETAIL_INDEX_SWITCH_BLOCK_4
#undef PTN_DETAIL_INDEX_SWITCH_CASE

  template <std::size_t N,
            typename Result,
            typename Arm,
            typename... Args>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  index_switch(std::size_t index, Args &&...args) {
    static_assert(N <= k_index_switch_max_arms,
                  "index_switch: too many arms");

    if constexpr (N <= k_index_switch_chain_max_arms) {
      if (PTN_DETAIL_UNLIKELY(index >= N)) {
        return Arm::template miss<Result>(
            std::forward<Args>(args)...);
      }
      return index_chain<0, N, Result, Arm>(
          index, std::forward<Args>(args)...);
    }
    else if constexpr (N <= 4) {
      return index_switch_4<N, Result, Arm>(
          index, std::forward<Args>(args)...);
    }
    else if constexpr (N <= 16) {
      return index_switch_16<N, Result, Arm>(
          index, std::forward<Args>(args)...);
    }
    else if constexpr (N <= 64) {
      return index_switch_64<N, Result, Arm>(
          index, std::forward<Args>(args)...);
    }
    else if constexpr (N <= 256) {
      return index_switch_256<N, Result, Arm>(
          index, std::forward<Args>(args)...);
    }
    else {
      return index_switch_512<N, Result, Arm>(
          index, std::forward<Args>(args)...);
    }
  }

} // namespace ptn::core::common::detail

#undef PTN_DETAIL_UNLIKELY
