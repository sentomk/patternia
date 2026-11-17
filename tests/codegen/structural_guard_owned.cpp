// Codegen probe: structural bindings with guard and handler
// expressions on an owned subject.
//
// The owned evaluation path passes continuations between its
// functions. GCC inlines a callee early only once it has processed
// it, so unless the whole path is always inlined, GCC 11 and 13 at
// -O2 kept the guards' constants on the stack and compared against
// memory (26 instructions against 13 for the same branches written
// by hand). See include/ptn/core/common/inline.hpp and
// scripts/check_codegen.py.

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
  return match(Point(*p))
         | on($(pt)[x > 0 && y > 0] >> x + y,
              $(pt)[x > 0] >> x - y,
              $(pt) >> x * x + y * y,
              _ >> 0);
}

extern "C" int ptn_codegen_reference(const Point *p) {
  if (p->x > 0 && p->y > 0) {
    return p->x + p->y;
  }
  if (p->x > 0) {
    return p->x - p->y;
  }
  return p->x * p->x + p->y * p->y;
}
