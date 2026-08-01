// Fuzz target for match evaluation.
//
// Strategy: oracle comparison. Every battery evaluates the same
// subject through patternia and through a hand-written reference
// implementation; any divergence traps. ASan/UBSan reports and
// divergences both count as failures.

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <variant>

#include <ptn/patternia.hpp>

using namespace ptn;

namespace {

  struct Point {
    int x;
    int y;
  };

  PTN_BIND(Point, x, y);

  [[noreturn]] void diverged(const char *battery) {
    // Print then trap: libFuzzer reports the input on abnormal
    // termination.
    std::fprintf(stderr, "oracle divergence in %s\n", battery);
    std::abort();
  }

  // --- literal dispatch battery ---

  int lit_ref(int v) {
    if (v == 0)
      return 10;
    if (v == 1)
      return 11;
    if (v == 2)
      return 12;
    if (v == 3)
      return 13;
    return -1;
  }

  int lit_match(int v) {
    return match(v)
           | on(             //
               lit(0) >> 10, //
               lit(1) >> 11, //
               val<2> >> 12, //
               val<3> >> 13, //
               _ >> -1);
  }

  // --- guard battery ---

  int guard_ref(int v) {
    if (v < 0)
      return 1;
    if (v >= 0 && v <= 9)
      return 2;
    if (v >= 10 && v < 100)
      return 3;
    return 4;
  }

  int guard_match(int v) {
    return match(v)
           | on(                           //
               $[_ < 0] >> 1,              //
               $[rng(0, 9)] >> 2,          //
               $[_ >= 10 && _ < 100] >> 3, //
               _ >> 4);
  }

  // --- structural battery ---

  int struct_ref(const Point &p) {
    if (p.x * p.x + p.y * p.y == 25)
      return p.x + p.y;
    if (p.x == 0 && p.y == 0)
      return 0;
    return -1;
  }

  int struct_match(const Point &p) {
    return match(p)
           | on( //
               $(has<&Point::x, &Point::y>)[x * x + y * y == 25] >>
                   [](int a, int b) { return a + b; },
               has<&Point::x, &Point::y>[x == 0 && y == 0] >> 0,
               _ >> -1);
  }

  // --- variant battery ---

  using Value = std::variant<int, std::string>;

  std::string variant_ref(const Value &v) {
    if (std::holds_alternative<int>(v))
      return "i:" + std::to_string(std::get<int>(v));
    return "s:" + std::get<std::string>(v);
  }

  std::string variant_match(const Value &v) {
    return match(v)
           | on( //
               $(is<int>) >>
                   [](int i) { return "i:" + std::to_string(i); },
               $(is<std::string>) >>
                   [](const std::string &s) { return "s:" + s; },
               _ >> [] { return std::string("?"); });
  }

  // --- combinator battery ---

  int comb_ref(int v) {
    if (v == 1 || v == 2 || v == 3)
      return 1;
    if (v != 0 && v % 2 == 0)
      return 2;
    return 3;
  }

  int comb_match(int v) {
    auto even = [](int n) { return n % 2 == 0; };
    return match(v)
           | on(                                  //
               (lit(1) || lit(2) || lit(3)) >> 1, //
               (!lit(0) && pred(even)) >> 2,      //
               _ >> 3);
  }

  // Reads sizeof(int) bytes starting at off (wrapping) as a
  // big-endian int.
  int read_int(const uint8_t *data, size_t size, size_t off) {
    int v = 0;
    for (size_t i = 0; i < sizeof(int); ++i) {
      v = (v << 8) | data[(off + i) % size];
    }
    return v;
  }

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data,
                                      size_t         size) {
  if (size < 8 || size > 4096)
    return 0;

  const int a = read_int(data, size, 0);
  const int b = read_int(data, size, 4);

  if (lit_match(a) != lit_ref(a))
    diverged("literal");
  if (guard_match(a) != guard_ref(a))
    diverged("guard");
  if (comb_match(b) != comb_ref(b))
    diverged("combinator");

  const Point p{a & 0xFF, b & 0xFF};
  if (struct_match(p) != struct_ref(p))
    diverged("structural");

  const size_t slen = static_cast<size_t>(b & 0x3F) % (size + 1);
  const Value  v    = (a & 1) ? Value{a}
                              : Value{std::string(
                                reinterpret_cast<const char *>(data),
                                slen)};
  if (variant_match(v) != variant_ref(v))
    diverged("variant");

  return 0;
}
