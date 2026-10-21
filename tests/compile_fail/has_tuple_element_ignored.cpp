// EXPECT_ERROR: [Patternia.has]: member index is out of range.
// FORBID_ERROR: incomplete type
// FORBID_ERROR: undefined template
#include <ptn/patternia.hpp>

struct Point {
  int x;
};

using pattern_t = std::decay_t<decltype(ptn::has<ptn::_ign>)>;
using invalid_t = std::tuple_element_t<0, pattern_t>;
