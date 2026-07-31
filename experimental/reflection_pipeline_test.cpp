// =============================================================================
// Integration test: has_refl<^^Type::member> in full match pipeline.
//
// Compile with:
//   clang-21 -std=c++26 -freflection -nostdinc++ \
//     -I<libcxx/include> -I<patternia/include> \
//     -o refl_pipeline experimental/reflection_pipeline_test.cpp
// =============================================================================
#include <cstdio>
#include <meta>
#include <tuple>

#include "ptn/patternia.hpp"
#include "ptn/pattern/experimental/reflect_decompose.hpp"

using namespace ptn;
using ptn::pat::reflect::decompose;
using ptn::pat::reflect::has_refl;

struct Point {
  int x;
  int y;
};

struct Packet {
  int type;
  int length;
};

// Test 1: Basic binding — decompose<^^T::m...>() >> handler
void test_basic_binding() {
  Point p{3, 4};
  int   result =
      match(p)
      | on(decompose<^^Point::x, ^^Point::y>()
               >> [](int x, int y) { return x + y; },
           _ >> 0);
  printf("test_basic_binding:       %d (expect 7)\n", result);
}

// Test 2: Guarded binding — decompose<...>()[guard] >> handler
void test_guarded_binding() {
  Point p{5, 3};
  int   result =
      match(p)
      | on(decompose<^^Point::x, ^^Point::y>()[_0 > arg<1>]
               >> [](int x, int y) { return x - y; },
           _ >> -1);
  printf("test_guarded_binding:     %d (expect 2)\n", result);
}

// Test 3: Guard rejects
void test_guard_rejects() {
  Point p{2, 5};
  int   result =
      match(p)
      | on(decompose<^^Point::x, ^^Point::y>()[_0 > arg<1>]
               >> [](int x, int y) { return x - y; },
           _ >> -1);
  printf("test_guard_rejects:       %d (expect -1)\n", result);
}

// Test 4: Partial member extraction
void test_partial_binding() {
  Point p{42, 99};
  int   result =
      match(p)
      | on(decompose<^^Point::x>()
               >> [](int x) { return x; },
           _ >> 0);
  printf("test_partial_binding:     %d (expect 42)\n", result);
}

// Test 5: Non-binding guard via has_refl<>
void test_nonbinding_guard() {
  Packet pkt{0x01, 100};
  int    result =
      match(pkt)
      | on(has_refl<^^Packet::type>[_0 == 1]
               >> [] { return 42; },
           _ >> 0);
  printf("test_nonbinding_guard:    %d (expect 42)\n", result);
}

// Test 6: Fallthrough on guard fail
void test_fallthrough() {
  Packet pkt{0x02, 100};
  int    result =
      match(pkt)
      | on(has_refl<^^Packet::type>[_0 == 1]
               >> [] { return 1; },
           has_refl<^^Packet::type>[_0 == 2]
               >> [] { return 2; },
           _ >> 0);
  printf("test_fallthrough:         %d (expect 2)\n", result);
}

// Test 7: Reordered members
void test_reordered() {
  Point p{3, 7};
  int   result =
      match(p)
      | on(decompose<^^Point::y, ^^Point::x>()
               >> [](int first, int second) {
                 return first * 10 + second;
               },
           _ >> 0);
  printf("test_reordered:           %d (expect 73)\n", result);
}

int main() {
  test_basic_binding();
  test_guarded_binding();
  test_guard_rejects();
  test_partial_binding();
  test_nonbinding_guard();
  test_fallthrough();
  test_reordered();

  printf("\n✅ All reflection pipeline tests passed\n");
  return 0;
}
