#include "ptn/patternia.hpp"

#include <string>
#include <variant>

namespace {
  using VariantT = std::variant<int, std::string>;

  int ct_combo_lit(int x) {
    using namespace ptn;
    return match(x)
           | on(ptn::lit(0) >> 0,
                ptn::lit(1) >> 1,
                ptn::lit(2) >> 2,
                ptn::lit(3) >> 3,
                ptn::lit(4) >> 4,
                _ >> 0);
  }

  int ct_combo_variant(const VariantT &v) {
    using namespace ptn;
    return match(v)
           | on(is<int>() >> 100, is<std::string>() >> 200, _ >> 0);
  }

  int (*volatile sink_lit)(int)              = ct_combo_lit;
  int (*volatile sink_var)(const VariantT &) = ct_combo_variant;
} // namespace
