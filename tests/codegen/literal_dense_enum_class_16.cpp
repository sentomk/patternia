// Codegen probe: 16 static literals on a scoped enum, plus a
// wildcard.
//
// `val<e>` on an `enum class` subject must take the same dense plan
// as on an integer (#112): a range check and a table or arithmetic.
// See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

// The default `int` base is the case from issue #112.
// NOLINTNEXTLINE(performance-enum-size)
enum class ProbeKey {
  k0,
  k1,
  k2,
  k3,
  k4,
  k5,
  k6,
  k7,
  k8,
  k9,
  k10,
  k11,
  k12,
  k13,
  k14,
  k15,
  other
};

#define PROBE_CASE(n) ptn::val<ProbeKey::k##n> >> ((n) * 3 + 1)

extern "C" int ptn_codegen_probe(ProbeKey x) {
  using namespace ptn;
  return match(x)
         | on(PROBE_CASE(0),
              PROBE_CASE(1),
              PROBE_CASE(2),
              PROBE_CASE(3),
              PROBE_CASE(4),
              PROBE_CASE(5),
              PROBE_CASE(6),
              PROBE_CASE(7),
              PROBE_CASE(8),
              PROBE_CASE(9),
              PROBE_CASE(10),
              PROBE_CASE(11),
              PROBE_CASE(12),
              PROBE_CASE(13),
              PROBE_CASE(14),
              PROBE_CASE(15),
              _ >> 0);
}

#define REFERENCE_CASE(n)                                           \
  case ProbeKey::k##n:                                              \
    return (n) * 3 + 1

// The same mapping written as a `switch`.
extern "C" int ptn_codegen_reference(ProbeKey x) {
  switch (x) {
    REFERENCE_CASE(0);
    REFERENCE_CASE(1);
    REFERENCE_CASE(2);
    REFERENCE_CASE(3);
    REFERENCE_CASE(4);
    REFERENCE_CASE(5);
    REFERENCE_CASE(6);
    REFERENCE_CASE(7);
    REFERENCE_CASE(8);
    REFERENCE_CASE(9);
    REFERENCE_CASE(10);
    REFERENCE_CASE(11);
    REFERENCE_CASE(12);
    REFERENCE_CASE(13);
    REFERENCE_CASE(14);
    REFERENCE_CASE(15);
  default:
    return 0;
  }
}
