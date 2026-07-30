// Reflection branch: prettier has<> syntax
// Goal: has<Point, "x", "y"> instead of has<&Point::x, &Point::y>

#include <cstdio>
#include <meta>
#include <array>
#include <string_view>
#include <tuple>
#include <utility>

// ============================================================
// FixedString for NTTP
// ============================================================
template <size_t N>
struct FixedString {
    char data[N];
    consteval FixedString(const char (&s)[N]) {
        for (size_t i = 0; i < N; ++i) data[i] = s[i];
    }
    consteval operator std::string_view() const {
        return std::string_view(data, N - 1);
    }
};
template <size_t N>
FixedString(const char (&)[N]) -> FixedString<N>;

// ============================================================
// Reflection helpers
// ============================================================
template <typename T>
consteval auto discover_members() {
    constexpr auto ctx = std::meta::access_context::current();
    return std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, ctx));
}

// Extract a single member value by name, via splice on the object.
// Each specialization for a specific name resolves the splice at compile time.
template <typename T, FixedString Name>
constexpr auto& get_named_member(T& obj) {
    constexpr auto members = discover_members<T>();
    template for (constexpr auto m : members) {
        if constexpr (std::meta::identifier_of(m) == std::string_view(Name)) {
            return obj.[:m:];
        }
    }
}

template <typename T, FixedString Name>
constexpr const auto& get_named_member(const T& obj) {
    constexpr auto members = discover_members<T>();
    template for (constexpr auto m : members) {
        if constexpr (std::meta::identifier_of(m) == std::string_view(Name)) {
            return obj.[:m:];
        }
    }
}
template <typename T, FixedString... Names>
struct has_refl {
    template <typename U>
    constexpr auto extract(U&& obj) const {
        // Fold-expression style: each get_named_member resolves at compile time
        return std::forward_as_tuple(
            get_named_member<std::remove_reference_t<U>, Names>(obj)...
        );
    }
};

// ============================================================
// Test
// ============================================================
struct Point { int x; int y; int z; };
struct Packet { int type; int len; };

int main() {
    Point p{3, 4, 5};

    // has<Point, "x", "y">
    auto h = has_refl<Point, "x", "y">{};
    auto [x, y] = h.extract(p);
    printf("x=%d y=%d\n", x, y);  // expect 3 4

    // has<Point, "x", "y", "z">
    auto h3 = has_refl<Point, "x", "y", "z">{};
    auto [a, b, c] = h3.extract(p);
    printf("x=%d y=%d z=%d\n", a, b, c);  // expect 3 4 5

    // Different type
    Packet pkt{0x01, 42};
    auto hp = has_refl<Packet, "type", "len">{};
    auto [t, l] = hp.extract(pkt);
    printf("type=%d len=%d\n", t, l);

    // Reordered members: has<Point, "y", "x">
    auto hr = has_refl<Point, "y", "x">{};
    auto [ry, rx] = hr.extract(p);
    printf("reordered: y=%d x=%d\n", ry, rx);  // expect 4 3

    printf("✅ has<T, \"name1\", \"name2\"> syntax works\n");
    return 0;
}
