#include <ptn/patternia.hpp>

struct Point {
  int x;
  int y;
};

// PTN_BIND names must be members of the given type; a misspelled
// member name must fail right at the declaration.
PTN_BIND(Point, xx);

int main() {
  return 0;
}
