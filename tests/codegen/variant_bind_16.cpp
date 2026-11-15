// Codegen probe: 16 variant alternatives, each bound and read.
//
// Binding the selected alternative must not cost more than the
// `std::get_if` a hand-written visitor would use: one dispatch on
// the index, then a load from the alternative.
// See scripts/check_codegen.py.

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
  return match(*subject)
         | on(
             $(is<Alt0>()) >>
                 [](const Alt0 &a) { return a.value + 11; },
             $(is<Alt1>()) >>
                 [](const Alt1 &a) { return a.value + 48; },
             $(is<Alt2>()) >>
                 [](const Alt2 &a) { return a.value + 85; },
             $(is<Alt3>()) >>
                 [](const Alt3 &a) { return a.value + 21; },
             $(is<Alt4>()) >>
                 [](const Alt4 &a) { return a.value + 58; },
             $(is<Alt5>()) >>
                 [](const Alt5 &a) { return a.value + 95; },
             $(is<Alt6>()) >>
                 [](const Alt6 &a) { return a.value + 31; },
             $(is<Alt7>()) >>
                 [](const Alt7 &a) { return a.value + 68; },
             $(is<Alt8>()) >>
                 [](const Alt8 &a) { return a.value + 4; },
             $(is<Alt9>()) >>
                 [](const Alt9 &a) { return a.value + 41; },
             $(is<Alt10>()) >>
                 [](const Alt10 &a) { return a.value + 78; },
             $(is<Alt11>()) >>
                 [](const Alt11 &a) { return a.value + 14; },
             $(is<Alt12>()) >>
                 [](const Alt12 &a) { return a.value + 51; },
             $(is<Alt13>()) >>
                 [](const Alt13 &a) { return a.value + 88; },
             $(is<Alt14>()) >>
                 [](const Alt14 &a) { return a.value + 24; },
             $(is<Alt15>()) >>
                 [](const Alt15 &a) { return a.value + 61; },
             _ >> 0);
}
