#pragma once
/**
 * @file meta.hpp
 * @brief Core compile-time utilities forming the foundation of the
 *        Patternia Type Layer (type-list, indexing, folds, traits).
 *
 * This header is NOT part of the Pattern DSL. It is internal meta
 * infrastructure used by type-patterns, variant-patterns, and template
 * introspection layers.
 */

#include <type_traits>
#include <cstddef>

namespace ptn::meta {

  //==========================================================================
  //  type_list<Ts...>
  //==========================================================================

  /**
   * @brief Simple compile-time container for a pack of types.
   *
   * Provides:
   *   - a `size` constant
   *   - a convenient representation for type-level computations
   */
  template <typename... Ts>
  struct type_list {
    static constexpr std::size_t size = sizeof...(Ts);
  };

  //==========================================================================
  //  remove_cvref_t
  //==========================================================================

  /**
   * @brief Remove const/volatile qualifiers and references from T.
   */
  template <typename T>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

  //==========================================================================
  //  concat / append / prepend
  //==========================================================================

  /**
   * @brief Concatenate two type_list sequences.
   */
  template <typename... A, typename... B>
  constexpr auto concat(type_list<A...>, type_list<B...>)
      -> type_list<A..., B...> {
    return {};
  }

  /**
   * @brief Append one type to a type_list.
   */
  template <typename T, typename... Ts>
  constexpr auto append(type_list<Ts...>) -> type_list<Ts..., T> {
    return {};
  }

  /**
   * @brief Prepend one type to a type_list.
   */
  template <typename T, typename... Ts>
  constexpr auto prepend(type_list<Ts...>) -> type_list<T, Ts...> {
    return {};
  }

  //==========================================================================
  //  contains<T, type_list<...>>
  //==========================================================================

  /**
   * @brief Check whether a type_list contains the type T.
   */
  template <typename T, typename TL>
  struct contains;

  template <typename T, typename... Ts>
  struct contains<T, type_list<Ts...>>
      : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

  template <typename T, typename TL>
  inline constexpr bool contains_v = contains<T, TL>::value;

  //==========================================================================
  //  index_of<T, type_list<...>>
  //==========================================================================

  /**
   * @brief Compile-time index of T in a type_list (or -1 if not found).
   *
   * Uses a constexpr loop for safe, deterministic behavior.
   */
  template <typename T, typename TL>
  struct index_of : std::integral_constant<int, -1> {};

  template <typename T, typename... Ts>
  struct index_of<T, type_list<Ts...>> {
  private:
    static constexpr int compute() {
      constexpr bool matches[] = {std::is_same_v<T, Ts>...};
      for (int i = 0; i < static_cast<int>(sizeof...(Ts)); ++i)
        if (matches[i])
          return i;
      return -1;
    }

  public:
    static constexpr int value = compute();
  };

  template <typename T, typename TL>
  inline constexpr int index_of_v = index_of<T, TL>::value;

  //==========================================================================
  //  nth_type<N, type_list<...>>
  //==========================================================================

  /**
   * @brief Retrieve the N-th type from a type_list.
   */
  template <std::size_t N, typename TL>
  struct nth_type;

  template <std::size_t N, typename T, typename... Ts>
  struct nth_type<N, type_list<T, Ts...>> {
    using type = typename nth_type<N - 1, type_list<Ts...>>::type;
  };

  template <typename T, typename... Ts>
  struct nth_type<0, type_list<T, Ts...>> {
    using type = T;
  };

  template <std::size_t N, typename TL>
  using nth_type_t = typename nth_type<N, TL>::type;

  //==========================================================================
  //  Fold predicates: all<...>, any<...>, none<...>
  //==========================================================================

  /**
   * @brief Logical AND fold over a pack of boolean template arguments.
   */
  template <bool... Bs>
  struct all : std::bool_constant<(Bs && ...)> {};

  /**
   * @brief Logical OR fold over a pack of boolean template arguments.
   */
  template <bool... Bs>
  struct any : std::bool_constant<(Bs || ...)> {};

  /**
   * @brief Logical NONE fold: true if all Bs are false.
   */
  template <bool... Bs>
  struct none : std::bool_constant<!any<Bs...>::value> {};

  //==========================================================================
  //  map<F, type_list<...>>
  //==========================================================================

  /**
   * @brief Apply a unary metafunction F<T> to every type in a type_list.
   */
  template <template <typename> typename F, typename TL>
  struct map;

  template <template <typename> typename F, typename... Ts>
  struct map<F, type_list<Ts...>> {
    using type = type_list<F<Ts>...>;
  };

  template <template <typename> typename F, typename TL>
  using map_t = typename map<F, TL>::type;

  //==========================================================================
  //  filter<Pred, type_list<...>>
  //==========================================================================

  /**
   * @brief Keep only types for which Pred<T>::value == true.
   */
  template <template <typename> typename Pred, typename TL>
  struct filter;

  template <template <typename> typename Pred>
  struct filter<Pred, type_list<>> {
    using type = type_list<>;
  };

  template <template <typename> typename Pred, typename T, typename... Ts>
  struct filter<Pred, type_list<T, Ts...>> {
  private:
    using tail = typename filter<Pred, type_list<Ts...>>::type;

  public:
    using type =
        std::conditional_t<Pred<T>::value, decltype(prepend<T>(tail{})), tail>;
  };

  template <template <typename> typename Pred, typename TL>
  using filter_t = typename filter<Pred, TL>::type;

  //==========================================================================
  //  is_spec_of & is_spec_of_v
  //==========================================================================

  /**
   * @brief Check whether T is a specialization of a given template.
   *
   * Only supports templates of the form `template<typename...> typename
   * Template`.
   */
  template <template <typename...> typename Template, typename T>
  struct is_spec_of : std::false_type {};

  template <template <typename...> typename Template, typename... Args>
  struct is_spec_of<Template, Template<Args...>> : std::true_type {};

  template <template <typename...> typename Template, typename T>
  inline constexpr bool is_spec_of_v = is_spec_of<Template, T>::value;

  //==========================================================================
  //  is_template_instance & is_template_instance_v
  //==========================================================================

  /**
   * @brief Detect whether a type is some template specialization at all
   *        (i.e. of the form Template<Args...> for some Template).
   *
   * This is intentionally lightweight and only supports typename templates
   * with type-parameter packs.
   */
  template <typename T>
  struct is_template_instance : std::false_type {};

  template <template <typename...> typename Template, typename... Args>
  struct is_template_instance<Template<Args...>> : std::true_type {};

  template <typename T>
  inline constexpr bool is_template_instance_v = is_template_instance<T>::value;

  //==========================================================================
  //  template_info
  //==========================================================================

  /**
   * @brief Extract template arguments and provide a `rebind` alias
   *        for a template specialization type.
   *
   * Usage:
   *   using info  = template_info<std::vector<int>>;
   *   using args  = info::args;                   // type_list<int>
   *   using VStr  = info::rebind<std::string>;    // std::vector<std::string>
   *
   * For non-template types, `is_specialization` is false and `args` is
   * an empty type_list<>.
   */
  template <typename T>
  struct template_info {
    static constexpr bool is_specialization = false;
    using args                              = type_list<>;
    // no rebind for non-template types
  };

  template <template <typename...> typename Template, typename... Args>
  struct template_info<Template<Args...>> {
    static constexpr bool is_specialization = true;
    using args                              = type_list<Args...>;

    template <typename... Us>
    using rebind = Template<Us...>;
  };

} // namespace ptn::meta
