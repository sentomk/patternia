#include <gtest/gtest.h>

#include <ptn/patternia.hpp>

#include <array>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

using namespace ptn;

namespace {
  template <std::size_t... I>
  auto sparse_dense_cases(std::index_sequence<I...>) {
    return on(
        (val<static_cast<int>(I * 4) - 256> >> static_cast<int>(
             I + 1))...,
        _ >> -1);
  }

  int literal_handler() {
    return 7;
  }
} // namespace

TEST(LiteralPattern, DenseDispatchCoversFullSpanAndGaps) {
  // Cross every 16-slot dispatch block, including negative keys.
  auto cases = sparse_dense_cases(std::make_index_sequence<128>{});
  using plan = ptn::core::common::detail::
      dispatch_plan<int, decltype(cases)::tuple_type>;
  static_assert(plan::kind
                == ptn::core::common::detail::dispatch_plan_kind::
                    static_literal_dense);

  for (int value = -258; value <= 254; ++value) {
    const int offset   = value + 256;
    const int expected = offset >= 0 && offset <= 508
                                 && offset % 4 == 0
                             ? offset / 4 + 1
                             : -1;
    EXPECT_EQ(match(value) | cases, expected) << value;
  }
}

TEST(LiteralPattern, ExplicitLiteralSupportsBuiltinHandlers) {
  auto values    = on(lit(0) >> 1, _ >> (1 << 2));
  auto functions = on(lit(0) >> &literal_handler, _ >> 0);
  int  zero      = 0;
  int  other     = 1;
  EXPECT_EQ(match(zero) | values, 1);
  EXPECT_EQ(match(other) | values, 4);
  EXPECT_EQ(match(zero) | functions, 7);
  EXPECT_EQ(match(other) | functions, 0);
}

TEST(LiteralPattern, LitMatchesExpectedValue) {
  int x      = 5;
  int result = ptn::match(x)
               | ptn::on(ptn::lit(5) >> 42, ptn::_ >> -1);

  EXPECT_EQ(result, 42);
}

TEST(LiteralPattern, LitOtherwiseFallback) {
  int x      = 3;
  int result = ptn::match(x)
               | ptn::on(ptn::lit(5) >> 42, ptn::_ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(LiteralPattern, LitMatchesStringLiteral) {
  std::string s      = "hello";
  int         result = ptn::match(s)
               | ptn::on(ptn::lit("hello") >> 1, ptn::_ >> 0);

  EXPECT_EQ(result, 1);
}

TEST(LiteralPattern, LitStringOtherwiseFallback) {
  std::string s      = "world";
  int         result = ptn::match(s)
               | ptn::on(ptn::lit("hello") >> 1, ptn::_ >> 0);

  EXPECT_EQ(result, 0);
}

TEST(LiteralPattern, LitMatchesCompileTimeValue) {
  int x      = 5;
  int result = ptn::match(x)
               | ptn::on(ptn::val<5> >> 42, ptn::_ >> -1);

  EXPECT_EQ(result, 42);
}

TEST(LiteralPattern, LitCompileTimeOtherwiseFallback) {
  int x      = 3;
  int result = ptn::match(x)
               | ptn::on(ptn::val<5> >> 42, ptn::_ >> -1);

  EXPECT_EQ(result, -1);
}

TEST(LiteralPattern, LitSelectsSparseDenseCases) {
  auto cases = on(ptn::val<1> >> 1,
                  ptn::val<2> >> 2,
                  ptn::val<10> >> 10,
                  _ >> 0);

  int one   = 1;
  int two   = 2;
  int ten   = 10;
  int other = 7;

  EXPECT_EQ(ptn::match(one) | cases, 1);
  EXPECT_EQ(ptn::match(two) | cases, 2);
  EXPECT_EQ(ptn::match(ten) | cases, 10);
  EXPECT_EQ(ptn::match(other) | cases, 0);
}

TEST(LiteralPattern, LitInlineOnSelectsDenseCases) {
  int one   = 1;
  int ten   = 10;
  int other = 7;

  int one_result = ptn::match(one)
                   | on(ptn::val<1> >> 1,
                        ptn::val<2> >> 2,
                        ptn::val<10> >> 10,
                        _ >> 0);
  int ten_result = ptn::match(ten)
                   | on(ptn::val<1> >> 1,
                        ptn::val<2> >> 2,
                        ptn::val<10> >> 10,
                        _ >> 0);
  int other_result = ptn::match(other)
                     | on(ptn::val<1> >> 1,
                          ptn::val<2> >> 2,
                          ptn::val<10> >> 10,
                          _ >> 0);

  EXPECT_EQ(one_result, 1);
  EXPECT_EQ(ten_result, 10);
  EXPECT_EQ(other_result, 0);
}

TEST(LiteralPattern, LitStaticOnFactorySelectsDenseCases) {
  int one   = 1;
  int ten   = 10;
  int other = 7;

  int one_result   = ptn::match(one) | ptn::static_on([] {
                     return on(ptn::val<1> >> 1,
                               ptn::val<2> >> 2,
                               ptn::val<10> >> 10,
                               _ >> 0);
                   });
  int ten_result   = ptn::match(ten) | ptn::static_on([] {
                     return on(ptn::val<1> >> 1,
                               ptn::val<2> >> 2,
                               ptn::val<10> >> 10,
                               _ >> 0);
                   });
  int other_result = ptn::match(other) | ptn::static_on([] {
                       return on(ptn::val<1> >> 1,
                                 ptn::val<2> >> 2,
                                 ptn::val<10> >> 10,
                                 _ >> 0);
                     });

  EXPECT_EQ(one_result, 1);
  EXPECT_EQ(ten_result, 10);
  EXPECT_EQ(other_result, 0);
}

TEST(LiteralPattern, LitPtnOnMacroSelectsDenseCases) {
  int one   = 1;
  int ten   = 10;
  int other = 7;

  int one_result = ptn::match(one)
                   | PTN_ON(ptn::val<1> >> 1,
                            ptn::val<2> >> 2,
                            ptn::val<10> >> 10,
                            _ >> 0);
  int ten_result = ptn::match(ten)
                   | PTN_ON(ptn::val<1> >> 1,
                            ptn::val<2> >> 2,
                            ptn::val<10> >> 10,
                            _ >> 0);
  int other_result = ptn::match(other)
                     | PTN_ON(ptn::val<1> >> 1,
                              ptn::val<2> >> 2,
                              ptn::val<10> >> 10,
                              _ >> 0);

  EXPECT_EQ(one_result, 1);
  EXPECT_EQ(ten_result, 10);
  EXPECT_EQ(other_result, 0);
}

TEST(LiteralPattern, LitCiMatchesAsciiCaseInsensitive) {
  std::string s      = "HeLLo";
  int         result = ptn::match(s)
               | ptn::on(ptn::lit_ci("hello") >> 1, ptn::_ >> 0);

  EXPECT_EQ(result, 1);
}

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)                  \
    || __cplusplus >= 202002L
TEST(LiteralPattern, ValSupportsFloatingPointInCpp20) {
  double x   = 3.25;
  int    hit = ptn::match(x)
            | ptn::on(ptn::val<3.25> >> 7, ptn::_ >> -1);

  EXPECT_EQ(hit, 7);
}
#endif

// --- lit_ci branch coverage: non-letter chars, size mismatch,
// and the generic convertible-to-string_view comparison path. ---

TEST(LiteralPattern, LitCiNonLetterCharacters) {
  // Exercises tolower_ascii's non-letter branch (digits/symbols).
  std::string s = "ABC-123_XYZ";
  auto        r = ptn::match(s)
           | ptn::on(ptn::lit_ci("abc-123_xyz") >> 1, ptn::_ >> 0);
  EXPECT_EQ(r, 1);
}

TEST(LiteralPattern, LitCiSizeMismatch) {
  // Different lengths hit the early-return false in iequal_ascii.
  std::string s = "ab";
  auto        r = ptn::match(s)
           | ptn::on(ptn::lit_ci("abc") >> 1, ptn::_ >> 0);
  EXPECT_EQ(r, 0);
}

TEST(LiteralPattern, LitCiAgainstStringViewSubject) {
  // Subject as std::string_view exercises the comparison through
  // the string_view overload.
  std::string_view s = "HeLLo";
  auto             r = ptn::match(s)
           | ptn::on(ptn::lit_ci("hello") >> 1, ptn::_ >> 0);
  EXPECT_EQ(r, 1);
}

TEST(LiteralPattern, LitCiCharLiteralSubject) {
  // const char* subject forces the templated convertible overload
  // of iequal_ascii::operator().
  const char *s = "World";
  auto        r = ptn::match(s)
           | ptn::on(ptn::lit_ci("WORLD") >> 1, ptn::_ >> 0);
  EXPECT_EQ(r, 1);
}

TEST(LiteralPattern, LitCiRejectsSameLengthMismatch) {
  for (std::string value : {"xBC", "AxC", "ABx"}) {
    EXPECT_EQ(match(value) | on(lit_ci("abc") >> 1, _ >> 0), 0);
  }
}

TEST(LiteralPattern,
     DenseIndexTableKeepsFirstDuplicateAndEmptySlots) {
  namespace detail = ptn::core::common::detail;
  using cases      = decltype(on(
      val<-2> >> 10, val<2> >> 20, val<-2> >> 30, _ >> -1));
  using plan       = detail::dispatch_plan<int, cases::tuple_type>;
  using metadata   = plan::metadata_t;
  static_assert(plan::kind
                == detail::dispatch_plan_kind::static_literal_dense);

  // Check the table builder independently of dispatch: duplicate
  // keys must keep the first source index, and gaps must stay empty
  // even when a default case exists. Check constant evaluation too.
  constexpr auto compiled = detail::
      make_static_literal_case_index_table<metadata,
                                           int,
                                           cases::tuple_type>();
  static_assert(compiled[0] == 0 && compiled[4] == 1);
  static_assert(compiled[2] == metadata::k_invalid_case_index);

  std::array<metadata::case_index_t, metadata::range_size> table;
  table.fill(metadata::k_invalid_case_index);
  detail::fill_static_literal_case_index_table<metadata,
                                               int,
                                               cases::tuple_type>(
      table);
  ASSERT_EQ(table.size(), 5u);
  EXPECT_EQ(table[0], 0);
  EXPECT_EQ(table[1], metadata::k_invalid_case_index);
  EXPECT_EQ(table[2], metadata::k_invalid_case_index);
  EXPECT_EQ(table[3], metadata::k_invalid_case_index);
  EXPECT_EQ(table[4], 1);
  EXPECT_EQ(table, compiled);
  EXPECT_EQ((detail::make_static_literal_case_index_table<
                metadata,
                int,
                cases::tuple_type>()),
            table);
}

TEST(LiteralPattern, LitChainSelectsFirstMatchingCase) {
  namespace detail = ptn::core::common::detail;
  const int three  = 3;
  auto      cases  = on(lit(1) >> 10,
                  lit(three) >> 20,
                  lit(3) >> 30,
                  lit(1) >> 40,
                  _ >> -1);
  using plan       = detail::dispatch_plan<int,
                                           decltype(cases)::tuple_type>;
  static_assert(plan::kind
                == detail::dispatch_plan_kind::literal_linear);

  // The selection runs before any handler, so duplicate values must
  // still resolve to the earliest case, on borrowed and owned
  // subjects alike.
  int one = 1;
  int x   = 3;
  int y   = 4;
  EXPECT_EQ(match(one) | cases, 10);
  EXPECT_EQ(match(x) | cases, 20);
  EXPECT_EQ(match(y) | cases, -1);
  EXPECT_EQ(match(1) | cases, 10);
  EXPECT_EQ(match(3) | cases, 20);
  EXPECT_EQ(match(4) | cases, -1);
}

TEST(LiteralPattern, LitChainRunsOnlyTheSelectedHandler) {
  int  calls[4] = {};
  int  fallback = 0;
  auto cases    = on(
      lit(5) >> [&] { return ++calls[0]; },
      lit(6) >> [&] { return ++calls[1]; },
      lit(5) >> [&] { return ++calls[2]; },
      lit(7) >> [&] { return ++calls[3]; },
      _ >> [&] { return -(++fallback); });

  for (int value : {5, 6, 7, 8, 5}) {
    match(value) | cases;
  }
  EXPECT_EQ(calls[0], 2);
  EXPECT_EQ(calls[1], 1);
  EXPECT_EQ(calls[2], 0);
  EXPECT_EQ(calls[3], 1);
  EXPECT_EQ(fallback, 1);
}

TEST(LiteralPattern, LitChainWithOnlyAFallback) {
  int value = 9;
  EXPECT_EQ(match(value) | on(_ >> 1), 1);
  EXPECT_EQ(match(9) | on(_ >> 2), 2);
}

TEST(LiteralPattern, CasePackIsTupleLike) {
  using pack = decltype(on(lit(1) >> 1, _ >> 2))::tuple_type;
  static_assert(std::tuple_size_v<pack> == 2);
  static_assert(std::tuple_size_v<const pack> == 2);
  static_assert(std::is_same_v<std::tuple_element_t<1, pack>,
                               std::decay_t<decltype(_ >> 2)>>);
}

TEST(LiteralPattern, CasePackKeepsValueCategoryOfHandlers) {
  // A temporary pack hands its handlers over as rvalues, so a
  // value handler can move its value out and a move-only capture
  // stays usable. A named pack is only read.
  int  value = 2;
  auto owned = match(value)
               | on(lit(1) >> std::string("one"),
                    lit(2) >> std::string("two"),
                    _ >> std::string("other"));
  EXPECT_EQ(owned, "two");

  auto pointer = std::make_unique<int>(42);
  EXPECT_EQ(
      match(value)
          | on(
              lit(2) >> [p = std::move(pointer)] { return *p; },
              _ >> 0),
      42);

  const auto named = on(lit(1) >> std::string("one"),
                        _ >> std::string("other"));
  EXPECT_EQ(match(value) | named, "other");
  int one = 1;
  EXPECT_EQ(match(one) | named, "one");
  EXPECT_EQ(match(one) | named, "one");
}
