#include "ptn/patternia.hpp"

#include <cassert>
#include <string>
#include <memory>
#include <iostream>

using namespace ptn;

struct Point {
  int x, y;
};

int main() {
  // —— 1. 整数匹配 ——
  {
    int  x   = 0;
    auto res = match(x)
                   .with([](int v) { return v == 0; }, [](int) { return std::string{"zero"}; })
                   .with([](int v) { return v > 0; }, [](int) { return std::string{"positive"}; })
                   .otherwise([](int) { return std::string{"negative"}; });
    assert(res == "zero");
  }
}