// test: tests_ptn.cpp

#include "ptn/patternia.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace ptn;

/**
 * @brief simple struct to demonstrate custom-type matching.
 */
struct Point {
  int x;
  int y;
};

int main() {
  /**
   * test 1: match integer values for zero, positive, and negative.
   */
  {
    int  value = 0;
    auto result =
        match(value)
            .with([](int v) { return v == 0; }, [](int) { return std::string{"zero"}; })
            .with([](int v) { return v > 0; }, [](int) { return std::string{"positive"}; })
            .otherwise([](int) { return std::string{"negative"}; });
    assert(result == "zero");
  }

  {
    int  value = 42;
    auto result =
        match(value)
            .with([](int v) { return v == 0; }, [](int) { return std::string{"zero"}; })
            .with([](int v) { return v > 0; }, [](int) { return std::string{"positive"}; })
            .otherwise([](int) { return std::string{"negative"}; });
    assert(result == "positive");
  }

  {
    int  value = -7;
    auto result =
        match(value)
            .with([](int v) { return v == 0; }, [](int) { return std::string{"zero"}; })
            .with([](int v) { return v > 0; }, [](int) { return std::string{"positive"}; })
            .otherwise([](int) { return std::string{"negative"}; });
    assert(result == "negative");
  }

  /**
   * test 2: match a custom struct Point by sums of coordinates.
   */
  {
    Point p1{0, 0};
    auto  r1 = match(p1)
                  .with([](const Point &pt) { return pt.x == 0 && pt.y == 0; },
                        [](Point) { return std::string{"origin"}; })
                  .with([](const Point &pt) { return pt.x == pt.y; },
                        [](Point) { return std::string{"diagonal"}; })
                  .otherwise([](Point) { return std::string{"other"}; });
    assert(r1 == "origin");

    Point p2{3, 3};
    auto  r2 = match(p2)
                  .with([](const Point &pt) { return pt.x == 0 && pt.y == 0; },
                        [](Point) { return std::string{"origin"}; })
                  .with([](const Point &pt) { return pt.x == pt.y; },
                        [](Point) { return std::string{"diagonal"}; })
                  .otherwise([](Point) { return std::string{"other"}; });
    assert(r2 == "diagonal");

    Point p3{2, 5};
    auto  r3 = match(p3)
                  .with([](const Point &pt) { return pt.x == 0 && pt.y == 0; },
                        [](Point) { return std::string{"origin"}; })
                  .with([](const Point &pt) { return pt.x == pt.y; },
                        [](Point) { return std::string{"diagonal"}; })
                  .otherwise([](Point) { return std::string{"other"}; });
    assert(r3 == "other");
  }

  /**
   * test 3: match string values by prefix.
   */
  {
    std::string s   = "hello world";
    auto        res = match(s)
                   .with([](const std::string &str) { return str.rfind("hello", 0) == 0; },
                         [](std::string) { return std::string{"greeting"}; })
                   .otherwise([](std::string) { return std::string{"unknown"}; });
    assert(res == "greeting");
  }

  std::cout << "all tests passed!\n";
  return 0;
}
