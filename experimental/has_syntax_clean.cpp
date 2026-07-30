// Test: named placeholders — minimal working version
#include <cstdio>
#include <type_traits>

template <int N>
struct placeholder {
    static constexpr int index = N;
    constexpr placeholder() {}
};

struct expr_tag {};

template <typename L, typename R, char Op>
struct bin_expr {
    static constexpr expr_tag tag{};
    L l; R r;
};

// is_expr: matches placeholder<N> and bin_expr<...>
template <typename T> struct is_expr : std::is_same<T, expr_tag> {};
template <int N>  struct is_expr<placeholder<N>> : std::true_type {};
template <typename L, typename R, char Op>
                  struct is_expr<bin_expr<L, R, Op>> : std::true_type {};
template <typename T> inline constexpr bool is_expr_v = is_expr<T>::value;

// Binary ops for expr operands
template <typename L, typename R, std::enable_if_t<is_expr_v<L> && is_expr_v<R>, int> = 0>
constexpr auto operator<(L l, R r) { return bin_expr<L, R, '<'>{l, r}; }

template <typename L, typename R, std::enable_if_t<is_expr_v<L> && is_expr_v<R>, int> = 0>
constexpr auto operator>(L l, R r) { return bin_expr<L, R, '>'>{l, r}; }

template <typename L, typename R, std::enable_if_t<is_expr_v<L> && is_expr_v<R>, int> = 0>
constexpr auto operator+(L l, R r) { return bin_expr<L, R, '+'>{l, r}; }

template <typename L, typename R, std::enable_if_t<is_expr_v<L> && is_expr_v<R>, int> = 0>
constexpr auto operator==(L l, R r) { return bin_expr<L, R, '='>{l, r}; }

// Also expr vs literal
template <typename L, std::enable_if_t<is_expr_v<L>, int> = 0>
constexpr auto operator>(L l, int r) { return bin_expr<L, int, '>'>{l, r}; }

// ============================================================
// PTN_BIND
// ============================================================
#define PTN_BIND_2(Type, m0, m1)             \
    inline constexpr placeholder<0> m0{};    \
    inline constexpr placeholder<1> m1{};

#define PTN_BIND_3(Type, m0, m1, m2)         \
    inline constexpr placeholder<0> m0{};    \
    inline constexpr placeholder<1> m1{};    \
    inline constexpr placeholder<2> m2{};

#define PTN_BIND_PICK(_1, _2, _3, NAME, ...) NAME
#define PTN_BIND(Type, ...) PTN_BIND_PICK(__VA_ARGS__, \
    PTN_BIND_3, PTN_BIND_2, 0)(Type, __VA_ARGS__)

// ============================================================
// HAS macro: HAS(Point, x, y) => has_pattern<&Point::x, &Point::y>
// ============================================================
template <auto... Ms>
struct has_pattern {};

#define HAS_1(Type, m0) has_pattern<&Type::m0>
#define HAS_2(Type, m0, m1) has_pattern<&Type::m0, &Type::m1>
#define HAS_3(Type, m0, m1, m2) has_pattern<&Type::m0, &Type::m1, &Type::m2>

#define HAS_PICK(_1, _2, _3, NAME, ...) NAME
#define HAS(Type, ...) HAS_PICK(__VA_ARGS__, \
    HAS_3, HAS_2, HAS_1, 0)(Type, __VA_ARGS__)

// ============================================================
// Test
// ============================================================
struct Point { int x; int y; int z; };

PTN_BIND(Point, x, y);

int main() {
    // Named placeholders
    auto e1 = x < y;
    auto e2 = x > 5;
    auto e3 = x + y;
    (void)e1; (void)e2; (void)e3;

    // HAS macro
    auto h1 = HAS(Point, x, y){};
    auto h2 = HAS(Point, x, y, z){};
    (void)h1; (void)h2;

    static_assert(std::is_same_v<
        decltype(HAS(Point, x, y){}),
        has_pattern<&Point::x, &Point::y>>);

    printf("✅ All syntax variants compile\n");
    return 0;
}
