#ifndef PTN_PATTERNS_RELATIONAL_HPP
#define PTN_PATTERNS_RELATIONAL_HPP

#include <functional>
#include <type_traits>
#endif

namespace ptn::patterns {
  template <typename T>
  using rel_store_t = std::decay_t<T>;

  /* less than  */
  template <typename V, typename Cmp = std::less<>>
  struct lt_pattern {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
  };

  /* less equal */
  template <typename V, typename Cmp = std::less<>>
  struct le {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};
    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(v, x))) {
      /* v < x <=> x >= v*/
      return cmp(v, x);
    }
  };

  /* greater than */
  template <typename V, typename Cmp = std::greater<>>
  struct gt {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};

    template <typename X>
    constexpr bool operator()(const X &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
  };
} // namespace ptn::patterns