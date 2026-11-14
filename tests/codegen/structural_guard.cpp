// Codegen probe: structural bindings with guards.
//
// Member bindings and guard expressions must compile to direct
// member reads. See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

namespace {
  struct Point {
    int x;
    int y;
  };
} // namespace

extern "C" int ptn_codegen_probe(const Point *p) {
  using namespace ptn;
  constexpr auto pt = has<&Point::x, &Point::y>;
  auto [x, y]       = pt;
  return match(*p)
         | on($(pt)[x > 0 && y > 0] >> x + y,
              $(pt)[x > 0] >> x - y,
              $(pt) >> x * x + y * y,
              _ >> 0);
}
