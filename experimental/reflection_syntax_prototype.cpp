// =============================================================================
// Patternia Reflection-Named Binding — Syntax Design Prototype (v2)
//
// Fixes: consteval-only type handling, local template issues.
// =============================================================================

#include <meta>
#include <cstdio>
#include <tuple>
#include <type_traits>

namespace ptn::reflect {

  // ── helpers ──────────────────────────────────────────────────────

  template <typename T>
  consteval auto members_of() {
    constexpr auto ctx = std::meta::access_context::current();
    return std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, ctx));
  }

  template <typename T>
  consteval size_t member_count_v() {
    return members_of<T>().size();
  }

  // ─────────────────────────────────────────────────────────────────
  // DESIGN A: auto-decompose into a tuple
  //
  //   auto t = decompose(point);   // std::tuple<int, int> = {p.x, p.y}
  //   auto& [x, y] = t;            // structured bindings
  //
  // Works for ANY member count via template-for splice expansion.
  // ─────────────────────────────────────────────────────────────────

  template <typename T, size_t... Is>
  constexpr auto decompose_impl(const T& obj, std::index_sequence<Is...>) {
    constexpr auto members = members_of<T>();
    return std::make_tuple(obj.[:members[Is]:]...);
  }

  template <typename T>
  constexpr auto decompose(const T& obj) {
    constexpr size_t N = member_count_v<T>();
    return decompose_impl<T>(obj, std::make_index_sequence<N>{});
  }

  // ─────────────────────────────────────────────────────────────────
  // DESIGN B: named guard lambda — std::apply on decomposed members
  //
  //   auto g = make_guard<Point>([](int x, int y) {
  //       return x*x + y*y < 25;
  //   });
  //   g.check(point);   // → true
  //
  // The lambda receives member values as named parameters.
  // This is the most practical design for real use.
  // ─────────────────────────────────────────────────────────────────

  template <typename T, typename Guard>
  struct named_guard {
    Guard g;

    constexpr bool check(const T& obj) const {
      // Decompose then std::apply — the guard lambda gets named args
      return std::apply(g, decompose(obj));
    }
  };

  template <typename T, typename Guard>
  constexpr auto make_guard(Guard&& g) {
    return named_guard<T, std::decay_t<Guard>>{std::forward<Guard>(g)};
  }

  // ─────────────────────────────────────────────────────────────────
  // DESIGN C: macro-based name injection
  //
  //   PTN_BIND(Point, x, y);
  //   // Now `x` and `y` are constexpr placeholders that splice to
  //   // Point::x and Point::y when evaluated against a Point.
  //
  //   auto guard = x*x + y*y < 25_ptn;   // expression template
  // ─────────────────────────────────────────────────────────────────

  // A named binding that carries its member reflection.
  // When dereferenced against a subject, it splices to the member access.
  template <std::meta::info R>
  struct named_binding {
    static constexpr std::meta::info reflection = R;

    // Evaluate against a subject object
    template <typename T>
    static constexpr auto get(const T& obj) {
      return obj.[:R:];
    }
  };

  // Macro: injects named bindings for a struct's members.
  // PTN_BIND(Point, x, y) expands to:
  //   inline constexpr named_binding<^^Point::x> x{};
  //   inline constexpr named_binding<^^Point::y> y{};
  //
  // Then in guard expressions:
  //   auto val = x.get(point);   // reads point.x via splice
  #define PTN_BIND(Type, ...) \
    PTN_BIND_IMPL(Type, __VA_ARGS__)

  #define PTN_BIND_IMPL_1(Type, m0) \
    inline constexpr ::ptn::reflect::named_binding<^^Type::m0> m0{}

  #define PTN_BIND_IMPL_2(Type, m0, m1) \
    inline constexpr ::ptn::reflect::named_binding<^^Type::m0> m0{}; \
    inline constexpr ::ptn::reflect::named_binding<^^Type::m1> m1{}

  #define PTN_BIND_IMPL_3(Type, m0, m1, m2) \
    inline constexpr ::ptn::reflect::named_binding<^^Type::m0> m0{}; \
    inline constexpr ::ptn::reflect::named_binding<^^Type::m1> m1{}; \
    inline constexpr ::ptn::reflect::named_binding<^^Type::m2> m2{}

  #define PTN_BIND_EXPAND(m, ...) m(__VA_ARGS__)
  #define PTN_BIND_GET(_1, _2, _3, NAME, ...) NAME
  #define PTN_BIND_IMPL(Type, ...) \
    PTN_BIND_EXPAND(PTN_BIND_GET(__VA_ARGS__, \
        PTN_BIND_IMPL_3, PTN_BIND_IMPL_2, PTN_BIND_IMPL_1, X), Type, __VA_ARGS__)

} // namespace ptn::reflect


// =============================================================================
// DEMONSTRATION
// =============================================================================

struct Point { int x; int y; };

// Inject named bindings for Point's members
PTN_BIND(Point, x, y);

int main() {
  Point p{3, 4};

  printf("=== Design A: auto-decompose into tuple ===\n");
  {
    auto t = ptn::reflect::decompose(p);
    auto& [x, y] = t;
    printf("  x=%d, y=%d, x*x+y*y=%d, <25? %s\n",
           x, y, x*x + y*y, (x*x + y*y < 25) ? "yes" : "no");
  }

  printf("\n=== Design B: named guard lambda ===\n");
  {
    auto g = ptn::reflect::make_guard<Point>(
        [](int x, int y) { return x*x + y*y < 25; });
    printf("  guard(3,4): %s\n", g.check(p) ? "match" : "no match");

    auto g2 = ptn::reflect::make_guard<Point>(
        [](int x, int y) { return x > 0 && y > 0; });
    printf("  positive(3,4): %s\n", g2.check(p) ? "match" : "no match");
  }

  printf("\n=== Design C: PTN_BIND named splices ===\n");
  {
    // x and y are now constexpr named_binding objects
    // They carry the compile-time reflection of Point::x and Point::y
    printf("  x.get(p) = %d (via [:^^Point::x:] splice)\n", x.get(p));
    printf("  y.get(p) = %d (via [:^^Point::y:] splice)\n", y.get(p));
  }

  printf("\n=== Reflection introspection ===\n");
  {
    constexpr size_t n = ptn::reflect::member_count_v<Point>();
    printf("  Point has %zu non-static data members\n", n);
  }

  return 0;
}
