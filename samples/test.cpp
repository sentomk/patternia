// Simple test program demonstrating Patternia pattern matching.
//
// This program shows basic usage of the Patternia library including
// literal patterns, wildcard patterns, and the match DSL.

#include <iostream>
#include "ptn/patternia.hpp"

using namespace ptn;

// Example enumeration for potential future testing
enum class Status {
  Pending,
  Running,
  Failed
};

int main() {
  // Test basic integer pattern matching with literals and wildcards
  Status x = Status::Running;

  auto res = match(x)
                 .when(lit(Status::Pending) >> "Pending")
                 .when(lit(Status::Failed) >> "Failed")
                 .when(__ >> "Other1")
                 .otherwise([] { return "Other"; });
  std::cout << res;
  return 0;
}
