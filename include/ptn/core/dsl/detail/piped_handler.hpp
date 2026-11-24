#pragma once

// `case | f`: passes the result of a case's handler to `f`.
//
// `p >> h | f` parses as `(p >> h) | f`, because `>>` binds tighter
// than `|`. The result is a case with the same pattern whose handler
// calls `h` with the pattern's bindings and then `f` with that
// result, only once the case is selected. It is shorthand for a
// lambda that does the same, and adds no other semantics.

#include <functional>
#include <type_traits>
#include <utility>

#include "ptn/core/common/common_traits.hpp"
#include "ptn/core/common/inline.hpp"
#include "ptn/core/dsl/detail/case_expr_impl.hpp"
#include "ptn/core/dsl/detail/expression_handler.hpp"
#include "ptn/pattern/base/pattern_base.hpp"

namespace ptn::core::dsl::detail {

  // Calls `f(args...)`, or `std::invoke` for a pointer to member.
  // `std::invoke` is not constexpr in C++17, so plain callables are
  // called directly and stay usable in constant evaluation.
  template <typename F, typename... Args>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  pipe_invoke(F &&f, Args &&...args) {
    if constexpr (std::is_member_pointer_v<std::decay_t<F>>) {
      return std::invoke(std::forward<F>(f),
                         std::forward<Args>(args)...);
    }
    else {
      return std::forward<F>(f)(std::forward<Args>(args)...);
    }
  }

  // The value the handler before `|` passes on. An expression
  // handler keeps a bare placeholder as a reference to the bound
  // value (see expression_handler::pipe_source), so `_ | f` does not
  // copy what `f` may only read.
  template <typename H, typename... Args>
  PTN_DETAIL_ALWAYS_INLINE constexpr decltype(auto)
  pipe_source(H &&h, Args &&...args) {
    if constexpr (is_expression_handler_v<std::decay_t<H>>) {
      return std::forward<H>(h).pipe_source(
          std::forward<Args>(args)...);
    }
    else {
      return pipe_invoke(std::forward<H>(h),
                         std::forward<Args>(args)...);
    }
  }

  // The value `f` returns. A reference into the handler's result,
  // which is a temporary, would dangle, so an rvalue reference
  // result is returned by value. An lvalue reference is kept: it
  // refers to something `f` can reach outside the temporary.
  template <typename R>
  using pipe_result_t = std::conditional_t<
      std::is_rvalue_reference_v<R>,
      std::remove_cv_t<std::remove_reference_t<R>>,
      R>;

  template <typename Inner, typename F>
  inline constexpr bool is_pipe_target_v = std::conditional_t<
      std::is_void_v<Inner>,
      std::false_type,
      std::is_invocable<F, Inner>>::value;

  // The result type of `f(h(args...))`, computed without
  // instantiating the call body, so that checking whether the piped
  // handler is invocable never reaches its assertions. When `f`
  // cannot take the result, unreachable_t stands in: it is neutral
  // in the match's common result type, so only the pipe diagnostic
  // is reported. MSVC deduces a `decltype(auto)` return type while
  // testing invocability and would report a second, handler
  // signature diagnostic.
  template <bool Valid, typename PipeRef, typename Inner>
  struct piped_result {
    using type = traits::detail::unreachable_t;
  };

  template <typename PipeRef, typename Inner>
  struct piped_result<true, PipeRef, Inner> {
    using type = pipe_result_t<std::invoke_result_t<PipeRef, Inner>>;
  };

  template <typename HandlerRef, typename PipeRef, typename... Args>
  struct piped_call {
    using inner_t = decltype(pipe_source(std::declval<HandlerRef>(),
                                         std::declval<Args>()...));
    static constexpr bool valid = is_pipe_target_v<inner_t, PipeRef>;
    using type = typename piped_result<valid, PipeRef, inner_t>::
        type;
  };

  // The handler of `case | f`. Its call operators accept exactly the
  // arguments the inner handler accepts, so handler classification
  // and binding checks see the same signature as without `| f`.
  // Whether `f` accepts the result is checked in the body, so a
  // mismatch reports the pipe diagnostic rather than a handler
  // signature error.
  template <typename Handler, typename Pipe>
  struct piped_handler {
    Handler handler;
    Pipe    pipe;

  private:
    // `T` with the value category and constness of `Self`, as a
    // member of `Self` would have. Spelled out from the template
    // arguments: the class is incomplete where the call operators'
    // return types are formed.
    template <typename Self, typename T>
    using member_like_t = std::conditional_t<
        std::is_lvalue_reference_v<Self>,
        std::conditional_t<
            std::is_const_v<std::remove_reference_t<Self>>,
            const T &,
            T &>,
        std::conditional_t<
            std::is_const_v<std::remove_reference_t<Self>>,
            const T &&,
            T &&>>;

    template <typename Self, typename... Args>
    using call_t = piped_call<member_like_t<Self, Handler>,
                              member_like_t<Self, Pipe>,
                              Args...>;

    template <typename Self, typename... Args>
    PTN_DETAIL_ALWAYS_INLINE static constexpr
        typename call_t<Self, Args...>::type
        call(Self &&self, Args &&...args) {
      using info_t   = call_t<Self, Args...>;
      using inner_t  = typename info_t::inner_t;
      using result_t = typename info_t::type;

      // The assertions stay in this body rather than in a helper: a
      // call, even to an empty function, that GCC has not inlined
      // by the time it converts the switch keeps a literal match
      // from becoming a lookup table or arithmetic.
      static_assert(!std::is_void_v<inner_t>,
                    "[Patternia.pipe]: the handler before | returns "
                    "void, so there is no result to pass to the "
                    "callable after it.");
      static_assert(
          std::is_void_v<inner_t> || info_t::valid,
          "[Patternia.pipe]: the callable after | cannot be "
          "called with the result of the handler before "
          "it.");
      if constexpr (!info_t::valid) {
        // Keep the diagnostic above from cascading.
        return result_t{};
      }
      else if constexpr (std::is_void_v<result_t>) {
        pipe_invoke(std::forward<Self>(self).pipe,
                    pipe_source(std::forward<Self>(self).handler,
                                std::forward<Args>(args)...));
      }
      else {
        return static_cast<result_t>(
            pipe_invoke(std::forward<Self>(self).pipe,
                        pipe_source(std::forward<Self>(self).handler,
                                    std::forward<Args>(args)...)));
      }
    }

  public:
    template <
        typename... Args,
        std::enable_if_t<std::is_invocable_v<Handler &, Args...>,
                         int> = 0>
    PTN_DETAIL_ALWAYS_INLINE constexpr
        typename call_t<piped_handler &, Args...>::type
        operator()(Args &&...args) & {
      return call(*this, std::forward<Args>(args)...);
    }

    template <typename... Args,
              std::enable_if_t<
                  std::is_invocable_v<const Handler &, Args...>,
                  int> = 0>
    PTN_DETAIL_ALWAYS_INLINE constexpr
        typename call_t<const piped_handler &, Args...>::type
        operator()(Args &&...args) const & {
      return call(*this, std::forward<Args>(args)...);
    }

    template <typename... Args,
              std::enable_if_t<std::is_invocable_v<Handler, Args...>,
                               int> = 0>
    PTN_DETAIL_ALWAYS_INLINE constexpr
        typename call_t<piped_handler, Args...>::type
        operator()(Args &&...args) && {
      return call(std::move(*this), std::forward<Args>(args)...);
    }
  };

  template <typename Handler>
  struct is_piped_handler : std::false_type {};

  template <typename Handler, typename Pipe>
  struct is_piped_handler<piped_handler<Handler, Pipe>>
      : std::true_type {};

  template <typename F>
  inline constexpr bool
      is_pipe_operand_v = !std::is_base_of_v<
                              ptn::pat::base::pattern_tag,
                              std::decay_t<F>>
                          && traits::detail::is_handler_like_v<F>;

  // `case | f`.
  template <typename Pattern, typename Handler, typename F>
  constexpr auto operator|(case_expr<Pattern, Handler> current,
                           F                         &&f) {
    static_assert(
        !std::is_base_of_v<ptn::pat::base::pattern_tag,
                           std::decay_t<F>>,
        "[Patternia.pipe]: the right side of | is a pattern. "
        "`case | f` passes the handler's result to a callable; "
        "combine patterns with `||` instead.");
    static_assert(
        std::is_base_of_v<ptn::pat::base::pattern_tag,
                          std::decay_t<F>>
            || traits::detail::is_handler_like_v<F>,
        "[Patternia.pipe]: the right side of | must be callable, "
        "such as a function, a lambda, a pointer to member or "
        "to<T>.");
    if constexpr (is_pipe_operand_v<F>) {
      using pipe_t = std::decay_t<F>;
      return case_expr<Pattern, piped_handler<Handler, pipe_t>>{
          std::move(current.pattern),
          piped_handler<Handler, pipe_t>{
              std::move(current.handler),
              pipe_t(std::forward<F>(f))}};
    }
    else {
      // Keep the operand diagnostic from cascading into the match.
      return current;
    }
  }

} // namespace ptn::core::dsl::detail
