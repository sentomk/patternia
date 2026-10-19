#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <string>
#include <type_traits>
#include <variant>

namespace {

  namespace detail = ptn::pat::detail;
  namespace common = ptn::core::common::detail;

  using canonical_is_int = detail::
      type_is_pattern<int, detail::no_subpattern>;
  using canonical_alt_0 = detail::
      type_alt_pattern<0, detail::no_subpattern>;

  using ptn::pat::base::canonical_pattern_t;

  // The factory objects behind `is<T>` and `alt<I>` canonicalize to
  // the pattern types the dispatch analyses recognise.
  static_assert(
      std::is_same_v<canonical_pattern_t<decltype(ptn::is<int>)>,
                     canonical_is_int>);
  static_assert(
      std::is_same_v<canonical_pattern_t<decltype(ptn::alt<0>)>,
                     canonical_alt_0>);

  // Other patterns are left alone.
  static_assert(std::is_same_v<canonical_pattern_t<canonical_is_int>,
                               canonical_is_int>);
  static_assert(std::is_same_v<canonical_pattern_t<decltype(ptn::_)>,
                               std::decay_t<decltype(ptn::_)>>);

  // A case built with `>>` stores the canonical pattern, so the
  // default spelling reaches the same fast path as `is<T>()`.
  using bare_is_case  = decltype(ptn::is<int> >> 1);
  using bare_alt_case = decltype(ptn::alt<0> >> 1);

  static_assert(
      std::is_same_v<bare_is_case::pattern_type, canonical_is_int>);
  static_assert(
      std::is_same_v<bare_alt_case::pattern_type, canonical_alt_0>);
  static_assert(common::is_simple_variant_dispatch_pattern_v<
                bare_is_case::pattern_type>);
  static_assert(common::is_simple_variant_dispatch_pattern_v<
                bare_alt_case::pattern_type>);

  // `$(is<T>)` wraps the canonical type as well.
  static_assert(std::is_same_v<
                decltype(ptn::$(ptn::is<int>)),
                detail::binding_as_pattern<void, canonical_is_int>>);

  // The call forms already produced canonical types.
  static_assert(
      std::is_same_v<decltype(ptn::is<int>()), canonical_is_int>);
  static_assert(
      std::is_same_v<decltype(ptn::alt<0>()), canonical_alt_0>);

} // namespace

TEST(CanonicalPatterns, BareAndCallFormsAgree) {
  using V = std::variant<int, double, std::string>;

  const V values[] = {V{1}, V{2.5}, V{std::string("x")}};

  for (const V &v : values) {
    int bare = ptn::match(v)
               | ptn::on(ptn::is<int> >> 1,
                         ptn::is<double> >> 2,
                         ptn::is<std::string> >> 3,
                         ptn::_ >> 0);

    int call = ptn::match(v)
               | ptn::on(ptn::is<int>() >> 1,
                         ptn::is<double>() >> 2,
                         ptn::is<std::string>() >> 3,
                         ptn::_ >> 0);

    int by_index_bare = ptn::match(v)
                        | ptn::on(ptn::alt<0> >> 1,
                                  ptn::alt<1> >> 2,
                                  ptn::alt<2> >> 3,
                                  ptn::_ >> 0);

    int by_index_call = ptn::match(v)
                        | ptn::on(ptn::alt<0>() >> 1,
                                  ptn::alt<1>() >> 2,
                                  ptn::alt<2>() >> 3,
                                  ptn::_ >> 0);

    EXPECT_EQ(bare, call);
    EXPECT_EQ(bare, by_index_bare);
    EXPECT_EQ(bare, by_index_call);
  }
}

TEST(CanonicalPatterns, BoundBareFormBindsTheAlternative) {
  std::variant<int, std::string> v = std::string("patternia");

  int bare = ptn::match(v)
             | ptn::on(
                 ptn::$(ptn::is<std::string>) >>
                     [](const std::string &s) {
                       return static_cast<int>(s.size());
                     },
                 ptn::_ >> -1);

  EXPECT_EQ(bare, 9);
}

TEST(CanonicalPatterns, FirstMatchStillWins) {
  std::variant<int, std::string> v = 5;

  int hits   = 0;
  int result = ptn::match(v)
               | ptn::on(
                   ptn::is<int> >>
                       [&] {
                         ++hits;
                         return 1;
                       },
                   ptn::is<int> >>
                       [&] {
                         ++hits;
                         return 2;
                       },
                   ptn::_ >> 0);

  EXPECT_EQ(result, 1);
  EXPECT_EQ(hits, 1);
}
