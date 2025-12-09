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

  Status s = Status::Running;

  auto result =
      match(s)
          .when(lit(Status::Pending) >> [] { return "pending state"; })
          .when(
              bind(lit(Status::Running)) >>
              [](int whole) {
                std::cout << "Captured (as int): " << whole << "\n";
                return "running state";
              })
          .when(
              bind() >>
              [](int v) {
                std::cout << "Fallback capturing whole subject (int): " << v
                          << "\n";
                return "captured fallback";
              })
          .otherwise([] { return "otherwise"; });

  std::cout << "Result = " << result << "\n";
}
