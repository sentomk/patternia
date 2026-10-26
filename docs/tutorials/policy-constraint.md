# Policy Constraint Matching

Policy logic is usually a sequence of overlapping conditions.
Patternia turns those rules into an ordered case list.

```cpp
enum class Role {
  Guest,
  User,
  Admin
};

struct Request {
  Role role;
  int  resource_id;
  bool read_only;
};
```

## Naming Domain Predicates

```cpp
auto is_public_resource = [](Role, int id) {
  return id >= 0 && id < 1000;
};
```

A callable guard receives every member listed in `has<...>`, in order. The
rule below lists `role` and `resource_id`, so the predicate takes both, even
though it only inspects the id.

## Declaring the Rules

```cpp
bool allowed(const Request &req) {
  using namespace ptn;
  constexpr auto request =
      has<&Request::role, &Request::resource_id, &Request::read_only>;
  auto [role, resource_id, read_only] = request;

  return match(req) | on(
    $(has<&Request::role>)[_ == Role::Admin] >> true,
    $(request)
        [role == Role::User && resource_id >= 0 && read_only == true] >> true,
    $(has<&Request::role, &Request::resource_id>)
        [role == Role::Guest && is_public_resource] >> true,
    _ >> false
  );
}
```

## Why This Helps

Each rule is self-contained:

- the binding states which fields matter
- the guard states why the rule applies
- the handler states the outcome

That keeps policy code readable even when ordering matters.

The names declared from `request` follow members, not positions, so the
third rule can use `role` with a pattern that lists only `role` and
`resource_id`.
