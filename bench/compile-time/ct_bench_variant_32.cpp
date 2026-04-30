#include "ptn/patternia.hpp"

#include <cstddef>
#include <variant>

namespace {
  template <std::size_t I>
  struct Token {
    int value;
  };

  using VAlt32 = std::variant<Token<0>,
                              Token<1>,
                              Token<2>,
                              Token<3>,
                              Token<4>,
                              Token<5>,
                              Token<6>,
                              Token<7>,
                              Token<8>,
                              Token<9>,
                              Token<10>,
                              Token<11>,
                              Token<12>,
                              Token<13>,
                              Token<14>,
                              Token<15>,
                              Token<16>,
                              Token<17>,
                              Token<18>,
                              Token<19>,
                              Token<20>,
                              Token<21>,
                              Token<22>,
                              Token<23>,
                              Token<24>,
                              Token<25>,
                              Token<26>,
                              Token<27>,
                              Token<28>,
                              Token<29>,
                              Token<30>,
                              Token<31>>;

  int ct_var_alt_route(const VAlt32 &v) {
    using namespace ptn;
    return match(v)
           | on(alt<0>() >> 1,
                alt<1>() >> 2,
                alt<2>() >> 3,
                alt<3>() >> 4,
                alt<4>() >> 5,
                alt<5>() >> 6,
                alt<6>() >> 7,
                alt<7>() >> 8,
                alt<8>() >> 9,
                alt<9>() >> 10,
                alt<10>() >> 11,
                alt<11>() >> 12,
                alt<12>() >> 13,
                alt<13>() >> 14,
                alt<14>() >> 15,
                alt<15>() >> 16,
                alt<16>() >> 17,
                alt<17>() >> 18,
                alt<18>() >> 19,
                alt<19>() >> 20,
                alt<20>() >> 21,
                alt<21>() >> 22,
                alt<22>() >> 23,
                alt<23>() >> 24,
                alt<24>() >> 25,
                alt<25>() >> 26,
                alt<26>() >> 27,
                alt<27>() >> 28,
                alt<28>() >> 29,
                alt<29>() >> 30,
                alt<30>() >> 31,
                alt<31>() >> 32,
                __ >> 0);
  }

  int (*volatile sink)(const VAlt32 &) = ct_var_alt_route;
} // namespace
