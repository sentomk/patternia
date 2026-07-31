// Integration test: the regular has<> and $ APIs accept reflected
// members.
//
// Compile with the Bloomberg P2996 Clang fork and -freflection.
#include <cstdio>
#include <meta>

#include "ptn/patternia.hpp"

using namespace ptn;

struct Point {
  int x;
  int y;
};

struct Packet {
  int type;
  int length;
};

int main() {
  Point point{3, 7};

  const int sum = match(point)
                  | on(
                      $(has<^^Point::x, ^^Point::y>) >>
                          [](int x, int y) { return x + y; },
                      _ >> 0);

  const int reordered = match(point)
                        | on(
                            $(has<^^Point::y, ^^Point::x>) >>
                                [](int y, int x) {
                                  return y * 10 + x;
                                },
                            _ >> 0);

  const int partial = match(point)
                      | on(
                          $(has<^^Point::x, _ign, ^^Point::y>) >>
                              [](int x, int y) {
                                return x * 10 + y;
                              },
                          _ >> 0);

  Packet    packet{2, 100};
  const int guarded = match(packet)
                      | on(
                          has<^^Packet::type>[_0 == 1] >>
                              [] { return 1; },
                          has<^^Packet::type>[_0 == 2] >>
                              [] { return 2; },
                          _ >> 0);

  std::printf("sum=%d reordered=%d partial=%d guarded=%d\n",
              sum,
              reordered,
              partial,
              guarded);

  return sum == 10 && reordered == 73 && partial == 37
                 && guarded == 2
             ? 0
             : 1;
}
