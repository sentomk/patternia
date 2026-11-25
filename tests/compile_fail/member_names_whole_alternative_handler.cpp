// EXPECT_ERROR: [Patternia.handler]: A member name in this
// EXPECT_ERROR: is<T>($(has<...>)) or alt<I>($(has<...>))
// FORBID_ERROR: [Patternia.guard]
// FORBID_ERROR: Handler signature does not match
#include <ptn/patternia.hpp>

#include <variant>

using namespace ptn;

struct Order {
  int qty;
  int unit;
};

struct Ping {};

using Event = std::variant<Order, Ping>;

constexpr auto order = has<&Order::qty, &Order::unit>;

int main() {
  auto [qty, unit] = order;
  Event e          = Order{2, 3};
  // $ around is<T>(...) binds the whole Order, so the has<...>
  // inside it binds no members for qty and unit to refer to.
  return match(e) | on($(is<Order>(order)) >> (qty * unit), _ >> 0);
}
