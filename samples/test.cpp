#include "ptn/patternia.hpp"
using namespace ptn;

enum class Opcode : int {
  Login  = 1,
  Logout = 2
};

class based_ptn {};

struct Grade {};

int main() {
  int  x   = 1;
  auto res = match<Opcode>(x)
                 .when(lit(Opcode::Login) >> "Login")
                 .when(lit(Opcode::Logout) >> "Logout")
                 .otherwise("None");
}