// Codegen probe: guarded cases on a two-alternative variant.
//
// The index switch selects the alternative, so inside its arm a case
// that only tests the alternative needs no further test, and its
// binding can be taken without checking the index again. The match
// must compile to about as many instructions as the same branches
// written by hand, which ptn_codegen_reference holds. Without that
// each arm tested the active index again and the match was three to
// four times the size of the reference. Older Clang versions spill a
// little more, so tests/CMakeLists.txt gives this probe extra slack.
// See scripts/check_codegen.py.

#include <ptn/patternia.hpp>

#include <string>
#include <variant>

namespace {
  using Subject = std::variant<int, std::string>;

  // A lambda rather than a function: a case stores a function as a
  // pointer, which GCC 13 does not inline at -O2, and the probe is
  // about the dispatch, not about calling the guard.
  constexpr auto long_string = [](const std::string &s) {
    return s.size() > 4;
  };
} // namespace

extern "C" int ptn_codegen_probe(const Subject &v) {
  using namespace ptn;
  return match(v)
         | on($(is<int>())[_ > 100] >> 10,
              is<int>() >> 1,
              $(is<std::string>())[long_string] >> 20,
              is<std::string>() >> 2,
              _ >> 0);
}

extern "C" int ptn_codegen_reference(const Subject &v) {
  if (std::holds_alternative<int>(v)) {
    if (*std::get_if<int>(&v) > 100) {
      return 10;
    }
    return 1;
  }
  if (std::holds_alternative<std::string>(v)) {
    if (long_string(*std::get_if<std::string>(&v))) {
      return 20;
    }
    return 2;
  }
  return 0;
}
