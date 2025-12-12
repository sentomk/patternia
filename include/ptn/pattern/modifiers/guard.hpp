// Guard modifiers and placeholder expressions for pattern matching.
//
// This file provides guard functionality that allows patterns to be
// combined with predicate conditions. It includes placeholder expressions
// for common comparison operations and the guarded_pattern wrapper that
// applies predicates to bound values.

#include <functional>
#include <type_traits>
#include <tuple>
#include <utility>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat::mod {

  //
  struct guard_predicate_tag {};

  template <typename T>
  inline constexpr bool is_guard_predicate_v =
      std::is_base_of_v<guard_predicate_tag, std::decay_t<T>>;

  //
  template <typename Op, typename RHS>
  struct binary_predicate : guard_predicate_tag {

    RHS rhs;

    template <typename T>
    constexpr bool operator()(T &&v) const noexcept(noexcept(Op{}(v, rhs))) {
      return Op{}(v, rhs);
    }
  };

  // Placeholder type for creating comparison expressions.
  //
  // This type enables the syntax `_ > 20`, `_ < 10`, etc. by
  // overloading comparison operators that return lambda predicates.
  struct placeholder_t {

    // >
    template <typename T>
    auto operator>(T &&rhs) const {
      return binary_predicate<std::greater<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // <
    template <typename T>
    auto operator<(T &&rhs) const {
      return binary_predicate<std::less<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // >=
    template <typename T>
    auto operator>=(T &&rhs) const {
      return binary_predicate<std::greater_equal<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // <=
    template <typename T>
    auto operator<=(T &&rhs) const {
      return binary_predicate<std::less_equal<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // ==
    template <typename T>
    auto operator==(T &&rhs) const {
      return binary_predicate<std::equal_to<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }

    // !=
    template <typename T>
    auto operator!=(T &&rhs) const {
      return binary_predicate<std::not_equal_to<>, std::decay_t<T>>{
          {}, std::forward<T>(rhs)};
    }
  };

  // Global placeholder instance for guard expressions.
  inline constexpr placeholder_t _{};

  //
  template <typename L, typename R>
  struct pred_and : guard_predicate_tag {
    L lhs;
    R rhs;

    template <typename T>
    constexpr bool operator()(T &&v) const {
      return lhs(v) && rhs(v);
    }
  };

  template <typename L, typename R>
  struct pred_or : guard_predicate_tag {
    L lhs;
    R rhs;

    template <typename T>
    constexpr bool operator()(T &&v) const {
      return lhs(v) || rhs(v);
    }
  };

  template <
      typename L,
      typename R,
      std::enable_if_t<
          is_guard_predicate_v<L> || is_guard_predicate_v<R>,
          int> = 0>
  constexpr auto operator&&(L &&l, R &&r) {
    return pred_and<std::decay_t<L>, std::decay_t<R>>{
        std::forward<L>(l), std::forward<R>(r)};
  }

  template <
      typename L,
      typename R,
      std::enable_if_t<
          is_guard_predicate_v<L> || is_guard_predicate_v<R>,
          int> = 0>
  constexpr auto operator||(L &&l, R &&r) {
    return pred_or<std::decay_t<L>, std::decay_t<R>>{
        std::forward<L>(l), std::forward<R>(r)};
  }

  //
  enum class range_mode {
    closed,
    open,
    open_closed,
    closed_open
  };

  struct closed_t {};
  struct open_t {};
  struct open_closed_t {};
  struct closed_open_t {};

  inline constexpr closed_t      closed{};
  inline constexpr open_t        open{};
  inline constexpr open_closed_t open_closed{};
  inline constexpr closed_open_t closed_open{};

  template <typename T>
  struct range_predicate : guard_predicate_tag {
    T          lo;
    T          hi;
    range_mode mode;

    template <typename U>
    constexpr bool operator()(U &&v) const {
      switch (mode) {
      case range_mode::closed:
        return lo <= v && v <= hi;
      case range_mode::open:
        return lo < v && v < hi;
      case range_mode::open_closed:
        return lo < v && v <= hi;
      case range_mode::closed_open:
        return lo <= v && v < hi;
      }
      return false;
    }
  };

  template <typename T>
  constexpr auto rng(T lo, T hi) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::closed};
  }

  template <typename T>
  constexpr auto rng(T lo, T hi, open_t) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::open};
  }

  template <typename T>
  constexpr auto rng(T lo, T hi, open_closed_t) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::open_closed};
  }

  template <typename T>
  constexpr auto rng(T lo, T hi, closed_open_t) {
    return range_predicate<std::decay_t<T>>{
        {}, std::forward<T>(lo), std::forward<T>(hi), range_mode::closed_open};
  }

  // Type trait to detect if a pattern is a binding pattern.
  //
  // This trait checks if a pattern has the `is_binding` static member,
  // indicating it can bind values for use in guard expressions.
  //
  // Template parameters:
  //   P: The pattern type to check.
  //   void: SFINAE parameter (internal use).
  template <typename P, typename = void>
  struct is_binding_pattern : std::false_type {};

  // Specialization for types that have the is_binding marker.
  template <typename P>
  struct is_binding_pattern<P, std::void_t<decltype(P::is_binding)>>
      : std::bool_constant<P::is_binding> {};

  // Helper variable template for is_binding_pattern trait.
  template <typename P>
  inline constexpr bool is_binding_pattern_v = is_binding_pattern<P>::value;

  // Pattern wrapper that applies a predicate guard to an inner pattern.
  //
  // This pattern first matches using the inner pattern, then applies
  // a predicate to the bound value. The pattern matches only if both
  // the inner pattern matches and the predicate returns true.
  //
  // Template parameters:
  //   Inner: The inner pattern type to match against.
  //   Pred: The predicate type to apply to the bound value.
  template <typename Inner, typename Pred>
  struct guarded_pattern
      : base::pattern_base<guarded_pattern<Inner, Pred>>,
        base::binding_pattern_base<guarded_pattern<Inner, Pred>> {
    Inner inner;
    Pred  pred;

    // Constructs a guarded pattern with an inner pattern and predicate.
    //
    // Uses perfect forwarding to efficiently move or copy the arguments.
    //
    // Template parameters:
    //   I: The inner pattern type (deduced).
    //   P: The predicate type (deduced).
    // Parameters:
    //   i: The inner pattern to use for matching.
    //   p: The predicate to apply to the bound value.
    template <typename I, typename P>
    constexpr guarded_pattern(I &&i, P &&p)
        : inner(std::forward<I>(i)), pred(std::forward<P>(p)) {
    }

    // Attempts to match the subject against the guarded pattern.
    //
    // First matches the subject using the inner pattern, then applies
    // the predicate to the first bound value. The pattern matches
    // only if both operations succeed.
    //
    // Template parameter:
    //   Subject: The type of the subject to match.
    // Parameters:
    //   s: The subject value to match against.
    // Returns: true if both the inner pattern matches and the predicate returns
    // true.
    template <typename Subject>
    constexpr bool match(Subject &&s) const noexcept(
        noexcept(inner.match(std::forward<Subject>(s))) &&
        noexcept(inner.bind(std::forward<Subject>(s)))) {

      if (!inner.match(std::forward<Subject>(s)))
        return false;

      auto bound    = inner.bind(std::forward<Subject>(s));
      using bound_t = std::decay_t<decltype(bound)>;

      constexpr std::size_t N = std::tuple_size_v<bound_t>;

      if constexpr (ptn::pat::mod::is_guard_predicate_v<Pred>) {
        static_assert(
            N == 1,
            "[Patternia.guard]: Unary guard predicates (_ / rng / && / ||) "
            "require the pattern to bind exactly ONE value. "
            "For multi-value guards, use a callable predicate (lambda / "
            "where).");

        return static_cast<bool>(pred(std::get<0>(bound)));
      }
      else {
        // callable guard: lambda / where / custom functor
        return static_cast<bool>(std::apply(pred, bound));
      }
    }

    // Binds the subject using the inner pattern's binding logic.
    //
    // This method delegates to the inner pattern's bind method,
    // preserving the same binding semantics.
    //
    // Template parameter:
    //   Subject: The type of the subject to bind.
    // Parameters:
    //   subject: The subject value to bind.
    // Returns: The binding result from the inner pattern.
    template <typename Subject>
    constexpr decltype(auto) bind(Subject &&subject) const
        noexcept(noexcept(inner.bind(std::forward<Subject>(subject)))) {
      return inner.bind(std::forward<Subject>(subject));
    }
  };

} // namespace ptn::pat::mod

namespace ptn::pat::base {

  // Binding contract specialization for guarded_pattern.
  //
  // This specialization declares that a guarded_pattern binds the same
  // arguments as its inner pattern, effectively forwarding the binding
  // contract from the inner pattern.
  //
  // Template parameters:
  //   Inner: The inner pattern type.
  //   Pred: The predicate type (unused in binding).
  //   Subject: The subject type being matched.
  template <typename Inner, typename Pred, typename Subject>
  struct binding_args<mod::guarded_pattern<Inner, Pred>, Subject> {
    using type = typename binding_args<Inner, Subject>::type;
  };
} // namespace ptn::pat::base
