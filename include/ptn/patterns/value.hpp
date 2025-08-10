#ifndef VALUE_HPP
#define VALUE_HPP

#include <functional>
#include <string_view>
#include <type_traits>
namespace ptn::patterns {
  template <typename V, typename Cmp = std::equal_to<>>
  struct value_pattern {
    V   v;
    Cmp cmp{};
    template <typename X>
    constexpr bool operator()(X const &x) const noexcept(noexcept(cmp(x, v))) {
      return cmp(x, v);
    }
  };

  template <typename V>
  using value_sort_t = std::conditional_t<std::is_array_v<std::remove_reference_t<V>> ||
                                              std::is_same_v<std::decay_t<V>, const char *>,
                                          std::string_view,
                                          std::decay_t<V>>;

  template <typename V>
  constexpr auto value(V &&v) {
    using Store = value_sort_t<V>;
    return value_pattern<Store>{Store(std::forward<V>(v))};
  }

}; // namespace ptn::patterns

#endif