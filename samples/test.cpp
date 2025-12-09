#include <iostream>
#include "ptn/patternia.hpp"

using namespace ptn;

enum Status {
  Pending,
  Running,
  Completed,
  Failed
};

int main() {
  Status status{Status::Running};

  match(status)
      .when(lit(Status::Pending) >> [] { std::cout << "Pending"; })
      .otherwise([] { std::cout << "None"; });
}