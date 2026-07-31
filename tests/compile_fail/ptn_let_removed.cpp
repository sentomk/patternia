#include <ptn/patternia.hpp>

int main() {
  auto guard = PTN_LET(v, v == 1);
  (void) guard;
}
