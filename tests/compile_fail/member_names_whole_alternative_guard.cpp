// EXPECT_ERROR: [Patternia.guard]: A member name in this guard
// EXPECT_ERROR: is<T>($(has<...>)) or alt<I>($(has<...>))
// FORBID_ERROR: Member names are bound only by $(has<...>).
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
  return match(e) | on($(alt<0>(order))[qty > 0] >> 1, _ >> 0);
}
