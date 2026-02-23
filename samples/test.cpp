#include "ptn/patternia.hpp"
#include <iostream>
#include <tuple>
#include <utility>
#include <variant>

using namespace ptn;

namespace {

  struct forwarding_probe_pattern
      : ptn::pat::base::pattern_base<forwarding_probe_pattern>,
        ptn::pat::base::binding_pattern_base<
            forwarding_probe_pattern> {
    static int lvalue_bind_calls;
    static int rvalue_bind_calls;

    template <typename Subject>
    constexpr bool match(const Subject &) const noexcept {
      return true;
    }

    auto bind(const int &subject) const {
      ++lvalue_bind_calls;
      return std::tuple<int>{subject};
    }

    auto bind(int &&subject) const {
      ++rvalue_bind_calls;
      return std::tuple<int>{subject};
    }
  };

  int forwarding_probe_pattern::lvalue_bind_calls = 0;
  int forwarding_probe_pattern::rvalue_bind_calls = 0;

} // namespace

namespace ptn::pat::base {

  template <typename Subject>
  struct binding_args<::forwarding_probe_pattern, Subject> {
    using type = std::tuple<int>;
  };

} // namespace ptn::pat::base

namespace {

  bool run_forwarding_regression_probe() {
    forwarding_probe_pattern::lvalue_bind_calls = 0;
    forwarding_probe_pattern::rvalue_bind_calls = 0;

    int x = 7;
    int r = match(x)
                .when(forwarding_probe_pattern{} >>
                      [](int v) { return v; })
                .otherwise(-1);

    const bool ok = r == 7
                    && forwarding_probe_pattern::lvalue_bind_calls
                           == 1
                    && forwarding_probe_pattern::rvalue_bind_calls
                           == 0;

    if (!ok) {
      std::cerr
          << "forwarding regression: expected lvalue bind path\n";
    }
    return ok;
  }

} // namespace

int main() {

  using Point  = int;
  using Height = int;

  using V = std::variant<Point, Height>;

  V v{std::in_place_index<1>, 23};

  auto res = match(v)
                 .when(alt<0>() >> "Point")
                 .when(alt<1>() >> "Height")
                 .when(ptn::__ >> "Other")
                 .end();

  std::cout << res << '\n';

  if (!run_forwarding_regression_probe()) {
    return 1;
  }

  std::cout << "forwarding probe: ok\n";
  return 0;
}
