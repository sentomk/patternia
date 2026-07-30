// Test: prettier has<> syntax via macro
// Goal: HAS(Point, x, y) -> has<&Point::x, &Point::y>

#include <type_traits>

// ============================================================
// Current syntax (explicit member pointers)
// ============================================================
template <auto... Ms>
struct has_pattern {};

// ============================================================
// Proposal: HAS() macro — write names, not pointers
// ============================================================

// Prepend &Type:: to each member name
#define PTN_ADDR(Type, m) , &Type::m

#define PTN_HAS_1(Type, m0) has_pattern<&Type::m0>
#define PTN_HAS_2(Type, m0, m1) has_pattern<&Type::m0, &Type::m1>
#define PTN_HAS_3(Type, m0, m1, m2) has_pattern<&Type::m0, &Type::m1, &Type::m2>

#define PTN_HAS_PICK(_1, _2, _3, NAME, ...) NAME
#define HAS(Type, ...) PTN_HAS_PICK(__VA_ARGS__, \
    PTN_HAS_3, PTN_HAS_2, PTN_HAS_1, 0)(Type, __VA_ARGS__)

// ============================================================
// Test
// ============================================================
struct Point { int x; int y; int z; };

// Old way:
using old_t = has_pattern<&Point::x, &Point::y>;

// New way:
using new_t = HAS(Point, x, y);
using new3_t = HAS(Point, x, y, z);

static_assert(std::is_same_v<old_t, new_t>, "types must match");

int main() {
    auto h = HAS(Point, x, y){};
    auto h3 = HAS(Point, x, y, z){};
    (void)h; (void)h3;
    return 0;
}
