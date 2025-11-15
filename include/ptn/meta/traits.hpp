#pragma once
/**
 * @file traits.hpp
 * @brief Common compile-time traits used across Patternia:
 *        remove_cvref_t, is_spec_of, is_tmpl, etc.
 */

#include <type_traits>

namespace ptn::meta {

  //====================================================================
  //  remove_cvref_t
  //====================================================================

  /**
   * @brief Remove const/volatile qualifiers and references from T.
   */
  template <typename T>
  using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

  //====================================================================
  //  is_spec_of & is_spec_of_v
  //====================================================================

  /**
   * @brief Check whether T is a specialization of a given template.
   *
   * Only supports templates of the form:
   *   template<typename...> typename Template
   */
  template <template <typename...> typename Template, typename T>
  struct is_spec_of : std::false_type {};

  template <template <typename...> typename Template, typename... Args>
  struct is_spec_of<Template, Template<Args...>> : std::true_type {};

  /**
   * @brief Convenience value alias for is_spec_of.
   */
  template <template <typename...> typename Template, typename T>
  inline constexpr bool is_spec_of_v = is_spec_of<Template, T>::value;

  //====================================================================
  //  is_tmpl & is_tmpl_v
  //====================================================================

  /**
   * @brief Detect whether a type is some template instantiation of the form
   *        Template<Args...> for some Template.
   *
   * This is intentionally lightweight and only supports type templates
   * with type-parameter packs.
   */
  template <typename T>
  struct is_tmpl : std::false_type {};

  template <template <typename...> typename Template, typename... Args>
  struct is_tmpl<Template<Args...>> : std::true_type {};

  /**
   * @brief Convenience value alias for is_tmpl.
   */
  template <typename T>
  inline constexpr bool is_tmpl_v = is_tmpl<T>::value;

  //====================================================================
  //  Legacy compatibility aliases
  //====================================================================

  /**
   * @brief Backward-compatible alias for is_tmpl.
   *
   * Kept for users who relied on the old name `is_template_instance`.
   */
  template <typename T>
  using is_template_instance = is_tmpl<T>;

  /**
   * @brief Backward-compatible alias for is_tmpl_v.
   */
  template <typename T>
  inline constexpr bool is_template_instance_v = is_tmpl_v<T>;

} // namespace ptn::meta
