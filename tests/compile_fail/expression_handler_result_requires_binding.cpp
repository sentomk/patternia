// EXPECT_ERROR: [Patternia.match] Handler signature does not match
// EXPECT_ERROR: the pattern's binding result.
// FORBID_ERROR: no matching function for call to
// 'get_case_result_impl'
#include <ptn/patternia.hpp>

using namespace ptn;

// Return-type deduction must diagnose this without relying on the
// validation statements in match(). MSVC instantiates it first.
using result = core::traits::case_result_t<int &,
                                           decltype(_ >> _ + 1)>;

int main() {
}
