// Codegen probe: 16 variant alternatives of an owned subject, each
// bound by value.
//
// `match(std::move(v))` must dispatch on the active alternative like
// a borrowed match does, rather than test the alternatives one by
// one in source order: GCC compiled that to a chain of 15
// comparisons. `ptn_codegen_reference` is the same match as a
// `switch` on `index()`, for comparison by hand. The probe does not
// compare with it, because some compilers lower that `switch` to a
// lookup table of addends and the match to a jump table of the same
// cost. See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

#include <variant>

namespace {
  struct Alt0 {
    int value;
  };
  struct Alt1 {
    int value;
  };
  struct Alt2 {
    int value;
  };
  struct Alt3 {
    int value;
  };
  struct Alt4 {
    int value;
  };
  struct Alt5 {
    int value;
  };
  struct Alt6 {
    int value;
  };
  struct Alt7 {
    int value;
  };
  struct Alt8 {
    int value;
  };
  struct Alt9 {
    int value;
  };
  struct Alt10 {
    int value;
  };
  struct Alt11 {
    int value;
  };
  struct Alt12 {
    int value;
  };
  struct Alt13 {
    int value;
  };
  struct Alt14 {
    int value;
  };
  struct Alt15 {
    int value;
  };

  using Subject = std::variant<Alt0,
                               Alt1,
                               Alt2,
                               Alt3,
                               Alt4,
                               Alt5,
                               Alt6,
                               Alt7,
                               Alt8,
                               Alt9,
                               Alt10,
                               Alt11,
                               Alt12,
                               Alt13,
                               Alt14,
                               Alt15>;
} // namespace

extern "C" int ptn_codegen_probe(const Subject *subject) {
  using namespace ptn;
  return match(Subject(*subject))
         | on(
             $(is<Alt0>()) >> [](Alt0 a) { return a.value + 11; },
             $(is<Alt1>()) >> [](Alt1 a) { return a.value + 48; },
             $(is<Alt2>()) >> [](Alt2 a) { return a.value + 85; },
             $(is<Alt3>()) >> [](Alt3 a) { return a.value + 21; },
             $(is<Alt4>()) >> [](Alt4 a) { return a.value + 58; },
             $(is<Alt5>()) >> [](Alt5 a) { return a.value + 95; },
             $(is<Alt6>()) >> [](Alt6 a) { return a.value + 31; },
             $(is<Alt7>()) >> [](Alt7 a) { return a.value + 68; },
             $(is<Alt8>()) >> [](Alt8 a) { return a.value + 4; },
             $(is<Alt9>()) >> [](Alt9 a) { return a.value + 41; },
             $(is<Alt10>()) >> [](Alt10 a) { return a.value + 78; },
             $(is<Alt11>()) >> [](Alt11 a) { return a.value + 14; },
             $(is<Alt12>()) >> [](Alt12 a) { return a.value + 51; },
             $(is<Alt13>()) >> [](Alt13 a) { return a.value + 88; },
             $(is<Alt14>()) >> [](Alt14 a) { return a.value + 24; },
             $(is<Alt15>()) >> [](Alt15 a) { return a.value + 61; },
             _ >> 0);
}

extern "C" int ptn_codegen_reference(const Subject *source) {
  Subject subject(*source);
  switch (subject.index()) {
  case 0:
    return std::get_if<0>(&subject)->value + 11;
  case 1:
    return std::get_if<1>(&subject)->value + 48;
  case 2:
    return std::get_if<2>(&subject)->value + 85;
  case 3:
    return std::get_if<3>(&subject)->value + 21;
  case 4:
    return std::get_if<4>(&subject)->value + 58;
  case 5:
    return std::get_if<5>(&subject)->value + 95;
  case 6:
    return std::get_if<6>(&subject)->value + 31;
  case 7:
    return std::get_if<7>(&subject)->value + 68;
  case 8:
    return std::get_if<8>(&subject)->value + 4;
  case 9:
    return std::get_if<9>(&subject)->value + 41;
  case 10:
    return std::get_if<10>(&subject)->value + 78;
  case 11:
    return std::get_if<11>(&subject)->value + 14;
  case 12:
    return std::get_if<12>(&subject)->value + 51;
  case 13:
    return std::get_if<13>(&subject)->value + 88;
  case 14:
    return std::get_if<14>(&subject)->value + 24;
  case 15:
    return std::get_if<15>(&subject)->value + 61;
  default:
    return 0;
  }
}
