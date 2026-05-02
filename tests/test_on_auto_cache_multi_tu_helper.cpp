#include "auto_cache_shared_handlers.hpp"
#include "ptn/patternia.hpp"

using namespace ptn;
using namespace auto_cache_shared;

const void *tu_a_last_addr = nullptr;

extern "C" int tu_a_match(int x) {
#ifdef PTN_TESTING
  int result     = match(x) | on($ >> DoubleIt{}, _ >> ReturnZero{});
  tu_a_last_addr = ptn::core::engine::detail::
      ptn_testing_last_cached_addr;
  return result;
#else
  (void) x;
  return 0;
#endif
}
