#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "ptn/core/common/diagnostics.hpp"
#include "ptn/core/common/inline.hpp"

// Internal implementation of case expressions.
//
// This header contains the internal case expression structure that
// represents pattern-handler pairs created by the >> operator. This
// is an implementation detail and should not be used directly.

namespace ptn::core::dsl::detail {

  template <typename... Cases>
  struct on;

  // Internal representation of a pattern-handler pair (case).
  //
  // This struct is created implicitly when using the >> operator:
  // ```cpp
  // pattern >> handler  // Creates a case_expr internally.
  // ```
  //
  // Pattern is the pattern type to match against.
  // Handler is the handler type to execute on match.
  //
  // Internal implementation detail. Prefer using the >> operator.
  template <typename Pattern, typename Handler>
  struct case_expr {
    using pattern_type = Pattern;
    using handler_type = Handler;

#if defined(__cpp_no_unique_address)                                \
    && __cpp_no_unique_address >= 201803L
    [[no_unique_address]] Pattern
        pattern; // The pattern to match against.
    [[no_unique_address]] Handler
        handler; // The handler to execute if matched.
#else
    Pattern pattern; // The pattern to match against.
    Handler handler; // The handler to execute if matched.
#endif
  };

  template <typename T>
  struct value_handler {
    T value;

    // Always inlined: a literal match is only lowered to a table
    // when every handler call has been inlined early, see
    // case_pack below.
    template <typename... Args,
              typename Value        = T,
              std::enable_if_t<std::is_copy_constructible_v<Value>,
                               int> = 0>
    PTN_DETAIL_ALWAYS_INLINE constexpr T
    operator()(Args &&...) const & {
      return value;
    }

    template <typename... Args>
    PTN_DETAIL_ALWAYS_INLINE constexpr T operator()(Args &&...) && {
      return std::move(value);
    }
  };

  template <typename Handler>
  struct is_value_handler : std::false_type {};

  template <typename T>
  struct is_value_handler<value_handler<T>> : std::true_type {};

  template <typename Handler>
  inline constexpr bool is_value_handler_v = is_value_handler<
      std::remove_cv_t<std::remove_reference_t<Handler>>>::value;

  template <typename T>
  struct is_on : std::false_type {};

  template <typename... Cases>
  struct is_on<on<Cases...>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_on_v = is_on<
      std::remove_cv_t<std::remove_reference_t<T>>>::value;

  // Storage for the cases of an `on(...)` pack.
  //
  // A tuple-like type whose element access is always inlined. GCC
  // turns a literal or variant match into a jump or lookup table
  // only if, by the time its early switch passes run, every access
  // to the case pack has been inlined and the cases are known
  // constants. `std::get` on a `std::tuple` goes through several
  // standard-library helpers that are inlined early or late
  // depending on the order in which the translation unit defines
  // functions, so the same match compiled to a table in one file
  // and to a chain of compares in another.
  //
  // Each case lives in its own base so that `case_get` reaches it
  // with a single static cast. `std::tuple_size` and
  // `std::tuple_element` are specialized below, so type-level code
  // treats a case_pack like a std::tuple.
  template <std::size_t I, typename Case>
  struct case_slot {
    Case value;
  };

  template <typename Indices, typename... Cases>
  struct case_pack_base;

  template <std::size_t... I, typename... Cases>
  struct case_pack_base<std::index_sequence<I...>, Cases...>
      : case_slot<I, Cases>... {
    constexpr case_pack_base() = default;

    template <typename... Exprs>
    constexpr explicit case_pack_base(Exprs &&...exprs)
        : case_slot<I, Cases>{std::forward<Exprs>(exprs)}... {
    }
  };

  template <typename... Cases>
  struct case_pack
      : case_pack_base<std::index_sequence_for<Cases...>, Cases...> {
    using case_pack_base<std::index_sequence_for<Cases...>,
                         Cases...>::case_pack_base;
  };

  // The I-th case of a case_pack, with the pack's value category.
  // The slot type is deduced from the pack's base class.
  template <std::size_t I, typename Case>
  PTN_DETAIL_ALWAYS_INLINE constexpr Case &
  case_get(case_slot<I, Case> &slot) noexcept {
    return slot.value;
  }

  template <std::size_t I, typename Case>
  PTN_DETAIL_ALWAYS_INLINE constexpr const Case &
  case_get(const case_slot<I, Case> &slot) noexcept {
    return slot.value;
  }

  template <std::size_t I, typename Case>
  PTN_DETAIL_ALWAYS_INLINE constexpr Case &&
  case_get(case_slot<I, Case> &&slot) noexcept {
    return static_cast<Case &&>(slot.value);
  }

  template <std::size_t I, typename Case>
  PTN_DETAIL_ALWAYS_INLINE constexpr const Case &&
  case_get(const case_slot<I, Case> &&slot) noexcept {
    return static_cast<const Case &&>(slot.value);
  }

  // Internal callers and tests may also hand the evaluator a plain
  // std::tuple of cases.
  template <std::size_t I, typename... Cases>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  case_get(std::tuple<Cases...> &cases) noexcept {
    return std::get<I>(cases);
  }

  template <std::size_t I, typename... Cases>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  case_get(const std::tuple<Cases...> &cases) noexcept {
    return std::get<I>(cases);
  }

  template <std::size_t I, typename... Cases>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  case_get(std::tuple<Cases...> &&cases) noexcept {
    return std::get<I>(std::move(cases));
  }

  template <std::size_t I, typename... Cases>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  case_get(const std::tuple<Cases...> &&cases) noexcept {
    return std::get<I>(std::move(cases));
  }

  // Compact case pack used by `match(x) | on(...)`.
  template <typename... Cases>
  struct on {
    using tuple_type = case_pack<Cases...>;

    tuple_type cases;

    template <typename... Exprs,
              typename = std::enable_if_t<sizeof...(Exprs)
                                          == sizeof...(Cases)>>
    constexpr explicit on(Exprs &&...exprs)
        : cases(std::forward<Exprs>(exprs)...) {
      (core::common::static_assert_case_expression<Cases>(), ...);
    }
  };

  template <typename... Cases>
  on(Cases &&...) -> on<std::decay_t<Cases>...>;

} // namespace ptn::core::dsl::detail

namespace std {

  template <typename... Cases>
  struct tuple_size<ptn::core::dsl::detail::case_pack<Cases...>>
      : integral_constant<size_t, sizeof...(Cases)> {};

  template <size_t I, typename... Cases>
  struct tuple_element<I,
                       ptn::core::dsl::detail::case_pack<Cases...>>
      : tuple_element<I, tuple<Cases...>> {};

} // namespace std
