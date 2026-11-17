#pragma once

// Dense index dispatch: the single place where Patternia turns
// "which arm" into control flow.
//
// `index_visit<N, Result>(index, hit, miss)` runs
// `hit(std::integral_constant<std::size_t, I>{})` for the arm
// `I == index` and `miss()` when `index >= N`. It is a real `switch`
// over `0..N-1`, so the compiler lowers it the way it lowers a
// hand-written one: a lookup table when every arm yields a constant,
// a jump table otherwise.
//
// `hit` and `miss` are lambdas of the caller that capture its
// subject and case pack by reference, and every step is forced
// inline, so nothing has to survive in memory between selecting an
// arm and running it. See inline.hpp for why that matters.

#include <cstddef>
#include <type_traits>
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
  // index_visit.
  constexpr std::size_t k_index_switch_max_arms = 512;

  // Up to this many arms, index_visit tests the index with `if`.
  constexpr std::size_t k_index_switch_chain_max_arms = 2;

  // The constant that selects arm `I`. It depends on `N` so that an
  // arm's type is only created when the arm exists: the switch
  // spells out 512 arms, and a non-dependent
  // `integral_constant<size_t, I>` there would be created for each
  // of them in every translation unit and written to its debug
  // information.
  template <std::size_t N, std::size_t I>
  using index_visit_arm = std::integral_constant<std::size_t,
                                                 I + 0 * N>;

  // Each arm is an instantiation of the caller's generic lambda,
  // whose debug name is `operator()<integral_constant<..., I>>`.
  // Arms used to be function templates whose arguments include the
  // case pack's type, and GCC writes that type into the debug name
  // of every instantiation: a 64-case dense literal match produced
  // 12 MB of debug information at -O2 -g.
#define PTN_DETAIL_INDEX_VISIT_CASE(n)                              \
  case (n):                                                         \
    if constexpr ((n) < N) {                                        \
      return hit(index_visit_arm<N, (n)>{});                        \
    }                                                               \
    else {                                                          \
      return miss();                                                \
    }

#define PTN_DETAIL_INDEX_VISIT_BLOCK_4(b)                           \
  PTN_DETAIL_INDEX_VISIT_CASE((b) + 0)                              \
  PTN_DETAIL_INDEX_VISIT_CASE((b) + 1)                              \
  PTN_DETAIL_INDEX_VISIT_CASE((b) + 2)                              \
  PTN_DETAIL_INDEX_VISIT_CASE((b) + 3)

#define PTN_DETAIL_INDEX_VISIT_BLOCK_16(b)                          \
  PTN_DETAIL_INDEX_VISIT_BLOCK_4((b) + 0)                           \
  PTN_DETAIL_INDEX_VISIT_BLOCK_4((b) + 4)                           \
  PTN_DETAIL_INDEX_VISIT_BLOCK_4((b) + 8)                           \
  PTN_DETAIL_INDEX_VISIT_BLOCK_4((b) + 12)

#define PTN_DETAIL_INDEX_VISIT_BLOCK_64(b)                          \
  PTN_DETAIL_INDEX_VISIT_BLOCK_16((b) + 0)                          \
  PTN_DETAIL_INDEX_VISIT_BLOCK_16((b) + 16)                         \
  PTN_DETAIL_INDEX_VISIT_BLOCK_16((b) + 32)                         \
  PTN_DETAIL_INDEX_VISIT_BLOCK_16((b) + 48)

#define PTN_DETAIL_INDEX_VISIT_BLOCK_256(b)                         \
  PTN_DETAIL_INDEX_VISIT_BLOCK_64((b) + 0)                          \
  PTN_DETAIL_INDEX_VISIT_BLOCK_64((b) + 64)                         \
  PTN_DETAIL_INDEX_VISIT_BLOCK_64((b) + 128)                        \
  PTN_DETAIL_INDEX_VISIT_BLOCK_64((b) + 192)

#define PTN_DETAIL_DEFINE_INDEX_VISIT(name, blocks)                 \
  template <std::size_t N,                                          \
            typename Result,                                        \
            typename Hit,                                           \
            typename Miss>                                          \
  PTN_DETAIL_ALWAYS_INLINE constexpr Result name(                   \
      std::size_t index, Hit &hit, Miss &miss) {                    \
    switch (index) {                                                \
      blocks;                                                       \
    default:                                                        \
      return miss();                                                \
    }                                                               \
  }

  PTN_DETAIL_DEFINE_INDEX_VISIT(index_visit_4,
                                PTN_DETAIL_INDEX_VISIT_BLOCK_4(0))
  PTN_DETAIL_DEFINE_INDEX_VISIT(index_visit_16,
                                PTN_DETAIL_INDEX_VISIT_BLOCK_16(0))
  PTN_DETAIL_DEFINE_INDEX_VISIT(index_visit_64,
                                PTN_DETAIL_INDEX_VISIT_BLOCK_64(0))
  PTN_DETAIL_DEFINE_INDEX_VISIT(index_visit_256,
                                PTN_DETAIL_INDEX_VISIT_BLOCK_256(0))
  PTN_DETAIL_DEFINE_INDEX_VISIT(
      index_visit_512,
      PTN_DETAIL_INDEX_VISIT_BLOCK_256(0)
          PTN_DETAIL_INDEX_VISIT_BLOCK_256(256))

#undef PTN_DETAIL_DEFINE_INDEX_VISIT
#undef PTN_DETAIL_INDEX_VISIT_BLOCK_256
#undef PTN_DETAIL_INDEX_VISIT_BLOCK_64
#undef PTN_DETAIL_INDEX_VISIT_BLOCK_16
#undef PTN_DETAIL_INDEX_VISIT_BLOCK_4
#undef PTN_DETAIL_INDEX_VISIT_CASE

  template <std::size_t N,
            typename Result,
            typename Hit,
            typename Miss>
  PTN_DETAIL_ALWAYS_INLINE constexpr Result
  index_visit(std::size_t index, Hit &hit, Miss &miss) {
    static_assert(N <= k_index_switch_max_arms,
                  "index_visit: too many arms");

    if constexpr (N == 0) {
      return miss();
    }
    else if constexpr (N <= k_index_switch_chain_max_arms) {
      if (PTN_DETAIL_UNLIKELY(index >= N)) {
        return miss();
      }
      if constexpr (N == 2) {
        if (index == 0) {
          return hit(std::integral_constant<std::size_t, 0>{});
        }
        return hit(std::integral_constant<std::size_t, 1>{});
      }
      else {
        return hit(std::integral_constant<std::size_t, 0>{});
      }
    }
    else if constexpr (N <= 4) {
      return index_visit_4<N, Result>(index, hit, miss);
    }
    else if constexpr (N <= 16) {
      return index_visit_16<N, Result>(index, hit, miss);
    }
    else if constexpr (N <= 64) {
      return index_visit_64<N, Result>(index, hit, miss);
    }
    else if constexpr (N <= 256) {
      return index_visit_256<N, Result>(index, hit, miss);
    }
    else {
      return index_visit_512<N, Result>(index, hit, miss);
    }
  }

} // namespace ptn::core::common::detail

#undef PTN_DETAIL_UNLIKELY
