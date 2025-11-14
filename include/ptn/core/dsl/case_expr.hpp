#pragma once

namespace ptn::dsl {

  /**
   * @brief Represents a single case in match() DSL: pattern + handler.
   */
  template <typename Pattern, typename Handler>
  struct case_expr {
    Pattern pattern;
    Handler handler;
  };

} // namespace ptn::dsl