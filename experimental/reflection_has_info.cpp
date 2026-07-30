// Reflection branch: has<> with info instead of strings
// Goal: has<^^Point::x, ^^Point::y> — native reflection, no string matching

#include <cstdio>
#include <meta>
#include <tuple>
#include <utility>

// ============================================================
// has_refl<info...> — takes reflection values, not strings
// ============================================================
template <std::meta::info... Members>
struct has_refl {
    static_assert((std::meta::is_nonstatic_data_member(Members) && ...),
                  "all template args must be non-static data members");

    template <typename U>
    constexpr auto extract(U&& obj) const {
        return std::forward_as_tuple(obj.[:Members:]...);
    }
};

// ============================================================
// Test
// ============================================================
struct Point { int x; int y; int z; };
struct Packet { int type; int len; };

int main() {
    Point p{3, 4, 5};

    // Native reflection syntax — no strings
    auto h = has_refl<^^Point::x, ^^Point::y>{};
    auto [x, y] = h.extract(p);
    printf("x=%d y=%d\n", x, y);

    // Three members
    auto h3 = has_refl<^^Point::x, ^^Point::y, ^^Point::z>{};
    auto [a, b, c] = h3.extract(p);
    printf("a=%d b=%d c=%d\n", a, b, c);

    // Different type
    Packet pkt{0x01, 42};
    auto hp = has_refl<^^Packet::type, ^^Packet::len>{};
    auto [t, l] = hp.extract(pkt);
    printf("type=%d len=%d\n", t, l);

    // Reordered
    auto hr = has_refl<^^Point::y, ^^Point::x>{};
    auto [ry, rx] = hr.extract(p);
    printf("reordered: y=%d x=%d\n", ry, rx);

    printf("✅ has<^^Type::member> syntax works\n");
    return 0;
}
