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
auto is_public_resource = [](int id) {
  return id >= 0 && id < 1000;
};
```

## Declaring the Rules

```cpp
bool allowed(const Request &req) {
  using namespace ptn;

  return match(req) | on(
    bind(has<&Request::role>())[arg<0> == Role::Admin] >> true,
    bind(has<&Request::role, &Request::resource_id, &Request::read_only>())
        [arg<0> == Role::User && arg<1> >= 0 && arg<2> == true] >> true,
    bind(has<&Request::role, &Request::resource_id>())
        [arg<0> == Role::Guest && is_public_resource] >> true,
    __ >> false
  );
}
```

## Why This Helps

Each rule is self-contained:

- the binding states which fields matter
- the guard states why the rule applies
- the handler states the outcome

That keeps policy code readable even when ordering matters.
