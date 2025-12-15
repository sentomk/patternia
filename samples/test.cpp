#include <iostream>
#include <ptn/patternia.hpp>

using namespace ptn;

struct Point {
  int x, y;
};

std::string classify_point(const Point &p) {
    return match(p)
        .when(bind(has<&Point::x, &Point::y>())[
                arg<0] == 0 && arg<1] == 0
            ] >> "origin")
            
        .when(bind(has<&Point::x, &Point::y>())[ 
                arg<0> > 0 && arg<1> > 0 &&
                rng(0, 100)(arg<0>) && rng(0, 100)(arg<1>)
            ] >> "first quadrant (bounded)")
            
        .when(bind(has<&Point::x, &Point::y>())[
                (arg<0] == 0 || arg[1] == 0) &&
                !(arg[0] == 0 && arg[1] == 0)
            ] >> "on axis")
            
        .otherwise([] {
    return "other"; });
}