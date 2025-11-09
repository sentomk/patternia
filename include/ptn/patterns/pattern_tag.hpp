#pragma once
#include <type_traits>

namespace ptn::patterns {

  // pattern_tag
  struct pattern_tag {
    static constexpr bool is_pattern = true;
  };

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
  template <typename P>
  concept pattern_like = std::derived_from<P, pattern_tag>;
#else
  template <typename P>
  struct is_pattern_like : std::is_base_of<pattern_tag, P> {};
#endif

} // namespace ptn::patterns
