// Codegen probe: 16 variant alternatives through PTN_ON.
//
// PTN_ON caches its case pack in a function-local static. The cached
// pack must compile to the same code as the same cases written with
// on(...), which ptn_codegen_reference holds. GCC turns a dispatch
// on a non-const static into a jump table with two instructions per
// alternative, where on(...) becomes a lookup table.
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
         | PTN_ON(is<Alt0>() >> 11,
                  is<Alt1>() >> 48,
                  is<Alt2>() >> 85,
                  is<Alt3>() >> 21,
                  is<Alt4>() >> 58,
                  is<Alt5>() >> 95,
                  is<Alt6>() >> 31,
                  is<Alt7>() >> 68,
                  is<Alt8>() >> 4,
                  is<Alt9>() >> 41,
                  is<Alt10>() >> 78,
                  is<Alt11>() >> 14,
                  is<Alt12>() >> 51,
                  is<Alt13>() >> 88,
                  is<Alt14>() >> 24,
                  is<Alt15>() >> 61,
                  _ >> 0);
}

extern "C" int ptn_codegen_reference(const Subject *subject) {
  using namespace ptn;
  return match(*subject)
         | on(is<Alt0>() >> 11,
              is<Alt1>() >> 48,
              is<Alt2>() >> 85,
              is<Alt3>() >> 21,
              is<Alt4>() >> 58,
              is<Alt5>() >> 95,
              is<Alt6>() >> 31,
              is<Alt7>() >> 68,
              is<Alt8>() >> 4,
              is<Alt9>() >> 41,
              is<Alt10>() >> 78,
              is<Alt11>() >> 14,
              is<Alt12>() >> 51,
              is<Alt13>() >> 88,
              is<Alt14>() >> 24,
              is<Alt15>() >> 61,
              _ >> 0);
}
