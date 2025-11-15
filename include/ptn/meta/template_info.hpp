#pragma once
/**
 * @file template_info.hpp
 * @brief Introspection helpers for template instantiations:
 *        extract arguments and provide a rebind alias.
 */

#include "ptn/meta/type_list.hpp"

namespace ptn::meta {

  //====================================================================
  //  template_info
  //====================================================================

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
