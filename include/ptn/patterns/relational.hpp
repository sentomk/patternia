#ifndef PTN_PATTERNS_RELATIONAL_HPP
#define PTN_PATTERNS_RELATIONAL_HPP

#include <functional>  // std::less<>
#include <type_traits> // std::decay_t
#include <utility>     // std::forward

namespace ptn::patterns {

  /* enhance this alias later to support string_view */
  template <typename T>
  using rel_store_t = std::decay_t<T>;

  // x < v
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

  // x <= v  <=>  !(v < x)
  template <typename V, typename Cmp = std::less<>>
  struct le_pattern {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};

    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(v, x))) {
      return !cmp(v, x);
    }
  };

  // x > v  <=>  (v < x)
  template <typename V, typename Cmp = std::less<>>
  struct gt_pattern {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};

    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(v, x))) {
      return cmp(v, x);
    }
  };

  // x >= v  <=>  !(x < v)
  template <typename V, typename Cmp = std::less<>>
  struct ge_pattern {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};

    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return !cmp(x, v);
    }
  };

  // x == v
  template <typename V, typename Cmp = std::equal_to<>>
  struct eq_pattern {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};

    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
  };
  // x != v
  template <typename V, typename Cmp = std::not_equal_to<>>
  struct ne_pattern {
    rel_store_t<V>            v;
    [[no_unique_address]] Cmp cmp{};

    template <typename X>
    constexpr bool operator()(X const &x) const
        noexcept(noexcept(std::declval<const Cmp &>()(x, v))) {
      return cmp(x, v);
    }
  };

  /* Factories */
  template <typename V>
  constexpr auto lt(V &&v) {
    return lt_pattern<rel_store_t<V>>{rel_store_t<V>(std::forward<V>(v))};
  }
  template <typename V>
  constexpr auto le(V &&v) {
    return le_pattern<rel_store_t<V>>{rel_store_t<V>(std::forward<V>(v))};
  }
  template <typename V>
  constexpr auto gt(V &&v) {
    return gt_pattern<rel_store_t<V>>{rel_store_t<V>(std::forward<V>(v))};
  }
  template <typename V>
  constexpr auto ge(V &&v) {
    return ge_pattern<rel_store_t<V>>{rel_store_t<V>(std::forward<V>(v))};
  }
  template <typename V>
  constexpr auto eq(V &&v) {
    return eq_pattern<rel_store_t<V>>{rel_store_t<V>(std::forward<V>(v))};
  }
  template <typename V>
  constexpr auto ne(V &&v) {
    return ne_pattern<rel_store_t<V>>{rel_store_t<V>(std::forward<V>(v))};
  }

  /*
    between:
    closed==true  -> [lo, hi] :  !(x < lo) && !(hi < x)
    closed==false -> (lo, hi) :  (lo < x) &&  (x < hi)
  */
  template <typename L, typename R, typename Cmp = std::less<>>
  struct between_pattern {
    rel_store_t<L>            lo;
    rel_store_t<R>            hi;
    bool                      closed;
    [[no_unique_address]] Cmp cmp{};

    template <typename X>
    constexpr bool operator()(X const &x) const noexcept(
        noexcept(std::declval<const Cmp &>()(x, lo)) &&
        noexcept(std::declval<const Cmp &>()(hi, x)) &&
        noexcept(std::declval<const Cmp &>()(lo, x)) &&
        noexcept(std::declval<const Cmp &>()(x, hi))) {
      if (closed) {
        return !cmp(x, lo) && !cmp(hi, x);
      }
      else {
        return cmp(lo, x) && cmp(x, hi);
      }
    }
  };

  template <typename L, typename R>
  constexpr auto between(L &&lo, R &&hi, bool closed = true) {
    return between_pattern<rel_store_t<L>, rel_store_t<R>>{
        rel_store_t<L>(std::forward<L>(lo)),
        rel_store_t<R>(std::forward<R>(hi)),
        closed};
  }

} // namespace ptn::patterns

#endif // PTN_PATTERNS_RELATIONAL_HPP
