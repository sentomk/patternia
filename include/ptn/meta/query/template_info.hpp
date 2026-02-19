#pragma once

// Introspection helpers for template instantiations.
// Extracts arguments and provides a rebind alias.

#include "ptn/meta/dsa/type_list.hpp"

namespace ptn::meta {

  // Primary template for non-specialized types.
  // Exposes an empty argument list and no rebind alias.
  template <typename T>
  struct template_info {
    static constexpr bool is_specialization = false;
    using args                              = type_list<>;
    // Intentionally no rebind alias for non-template types.
  };

  template <template <typename...> typename Template, typename... Args>
  struct template_info<Template<Args...>> {
    static constexpr bool is_specialization = true;
    using args                              = type_list<Args...>;

    template <typename... Us>
    using rebind = Template<Us...>;
  };

} // namespace ptn::meta
