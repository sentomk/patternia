#include <ptn/patternia.hpp>

int main() {
  auto guard = PTN_WHERE((x, y), x < y);
  (void) guard;
}
