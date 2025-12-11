// Guard modifiers and placeholder expressions for pattern matching.
//
// This file provides guard functionality that allows patterns to be
// combined with predicate conditions. It includes placeholder expressions
// for common comparison operations and the guarded_pattern wrapper that
// applies predicates to bound values.

#include <type_traits>
#include <utility>

#include "ptn/pattern/base/fwd.h"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::pat::mod {

  // Placeholder type for creating comparison expressions.
  //
  // This type enables the syntax `_ > 20`, `_ < 10`, etc. by
  // overloading comparison operators that return lambda predicates.
  struct placeholder_t {

    // Creates a greater-than predicate.
    //
    // Returns a lambda that checks if a value is greater than rhs.
    //
    // Template parameter:
    //   T: The type of the right-hand side value.
    // Parameters:
    //   rhs: The value to compare against.
    // Returns: A lambda predicate `(auto &&v) { return v > rhs; }`.
    template <typename T>
    auto operator>(T &&rhs) const {
      return [rhs = std::forward<T>(rhs)](auto &&v) { return v > rhs; };
    }

    // Creates a less-than predicate.
    //
    // Returns a lambda that checks if a value is less than rhs.
    //
    // Template parameter:
    //   T: The type of the right-hand side value.
    // Parameters:
    //   rhs: The value to compare against.
    // Returns: A lambda predicate `(auto &&v) { return v < rhs; }`.
    template <typename T>
    auto operator<(T &&rhs) const {
      return [rhs = std::forward<T>(rhs)](auto &&v) { return v < rhs; };
    }

    // Creates an equality predicate.
    //
    // Returns a lambda that checks if a value equals rhs.
    //
    // Template parameter:
    //   T: The type of the right-hand side value.
    // Parameters:
    //   rhs: The value to compare against.
    // Returns: A lambda predicate `(auto &&v) { return v == rhs; }`.
    template <typename T>
    auto operator==(T &&rhs) const {
      return [rhs = std::forward<T>(rhs)](auto &&v) { return v == rhs; };
    }

    // Creates an inequality predicate.
    //
    // Returns a lambda that checks if a value does not equal rhs.
    //
    // Template parameter:
    //   T: The type of the right-hand side value.
    // Parameters:
    //   rhs: The value to compare against.
    // Returns: A lambda predicate `(auto &&v) { return v != rhs; }`.
    template <typename T>
    auto operator!=(T &&rhs) const {
      return [rhs = std::forward<T>(rhs)](auto &&v) { return v != rhs; };
    }
  };

  // Global placeholder instance for guard expressions.
  //
  // Usage examples:
  //   - `_ > 20` - matches values greater than 20
  //   - `_ < 100` - matches values less than 100
  //   - `_ == 42` - matches values equal to 42
  inline constexpr placeholder_t _{};

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
    // Returns: true if both the inner pattern matches and the predicate returns true.
    template <typename Subject>
    bool match(const Subject &s) const {
      if (!inner.match(s))
        return false;

      auto tup = inner.bind(s);

      constexpr size_t N = std::tuple_size_v<decltype(tup)>;
      static_assert(
          N == 1,
          "[Patternia.guard]: Guard requires the pattern to bind exactly ONE "
          "value. "
          "Use named variables for multi-value patterns.");

      auto &&v = std::get<0>(tup);
      return static_cast<bool>(pred(v));
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
    auto bind(Subject &&subject) const {
      return inner.bind(subject);
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
