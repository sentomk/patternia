# Patternia 架构分析文档

## 1. match_builder 的核心机制

### 1.1 match_builder 如何持有 subject

`match_builder` 通过成员变量持有 subject：

```cpp
template <typename TV, typename... Cases>
class match_builder {
private:
  subject_type subject_;  // 持有要匹配的值
  cases_type   cases_;    // 持有所有 case 表达式的 tuple
public:
  static constexpr auto create(subject_type subject) {
    return match_builder{std::move(subject), cases_type{}};
  }
};
```

- `subject_type` 是 `TV`（通常是 `std::decay_t<T>`）
- `subject_` 通过移动语义存储传入的值，避免不必要的拷贝
- `cases_` 是 `std::tuple<Cases...>`，初始为空

### 1.2 when() 如何扩展 cases tuple

`when()` 方法通过 `std::tuple_cat` 扩展 cases tuple：

```cpp
template <typename CaseExpr>
constexpr auto when(CaseExpr &&expr) && {
  using new_case_type = std::decay_t<CaseExpr>;
  
  // 类型检查：确保是 case_expr
  static_assert(ptn::core::common::is_case_expr_v<new_case_type>,
                "Argument to .when() must be a case expression");
  
  // 类型检查：确保 handler 可调用
  static_assert(ptn::core::common::is_handler_invocable_v<new_case_type, subject_type>,
                "Handler signature does not match pattern's binding result");
  
  // 扩展 cases tuple
  auto new_cases = std::tuple_cat(
      std::move(cases_),
      std::tuple<new_case_type>(std::forward<CaseExpr>(expr)));
  
  using builder_t = match_builder<subject_type, Cases..., new_case_type>;
  return builder_t{std::move(subject_), std::move(new_cases)};
}
```

关键机制：
- 每次调用 `when()` 都创建新的 `match_builder` 类型，模板参数包含新增的 case
- 使用 `std::tuple_cat` 将新的 case_expr 添加到现有 tuple
- 提供 rvalue 和 lvalue 两个重载以支持链式调用

### 1.3 otherwise() 如何触发最终 eval

`otherwise()` 是终端操作，触发整个匹配过程：

```cpp
template <typename Otherwise>
constexpr decltype(auto) otherwise(Otherwise &&otherwise_handler) && {
  auto final_handler = [&]() {
    if constexpr (ptn::core::common::detail::is_value_like_v<OtherwiseDecayed>) {
      // 值类型：创建返回该值的 handler
      return [val = std::forward<Otherwise>(otherwise_handler)](auto &&...) -> OtherwiseDecayed { 
        return val; 
      };
    } else {
      // 函数类型：直接使用
      return std::forward<Otherwise>(otherwise_handler);
    }
  }();
  
  // 编译时验证整个 match 表达式的有效性
  ptn::core::common::static_assert_valid_match<subject_type, decltype(final_handler), Cases...>();
  
  // 调用核心评估逻辑
  return match_impl::eval(subject_, cases_, std::move(final_handler));
}
```

## 2. builder 最终如何执行 eval_cases

执行路径：
```
match_builder::otherwise()
├── match_impl::eval()
└── ptn::core::common::eval_cases()
    └── detail::eval_cases_impl<0>()
```

`match_impl` 是简单的转发层：

```cpp
struct match_impl {
  template <typename TV, typename CasesTuple, typename Otherwise>
  static constexpr decltype(auto) eval(TV &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
    return ptn::core::common::eval_cases(
        subject, cases, std::forward<Otherwise>(otherwise_handler));
  }
};
```

## 3. eval_cases_impl 递归匹配机制

### 3.1 递归匹配算法

```cpp
template <std::size_t I, typename Subject, typename CasesTuple, typename Otherwise, std::size_t N>
constexpr decltype(auto) eval_cases_impl(Subject &subject, CasesTuple &cases, Otherwise &&otherwise_handler) {
  if constexpr (I >= N) {
    // 基础情况：没有 case 匹配，调用 otherwise handler
    if constexpr (std::is_invocable_v<Otherwise, Subject &>) {
      return std::forward<Otherwise>(otherwise_handler)(subject);
    } else {
      return std::forward<Otherwise>(otherwise_handler)();
    }
  } else {
    auto &current_case = std::get<I>(cases);
    using case_t = std::remove_reference_t<decltype(current_case)>;
    
    if (case_matcher<case_t, Subject>::matches(current_case, subject)) {
      // 找到匹配，调用 handler
      return invoke_handler(current_case, std::forward<Subject>(subject));
    } else {
      // 递归到下一个 case
      return eval_cases_impl<I + 1>(subject, cases, std::forward<Otherwise>(otherwise_handler));
    }
  }
}
```

### 3.2 invoke_handler 如何真正调用 handler

```cpp
template <typename Case, typename Subject>
constexpr decltype(auto) invoke_handler(const Case &c, Subject &&subject) {
  using pattern_type = case_pattern_t<Case>;
  
  // 让 pattern 执行绑定
  auto bound_values = c.pattern.bind(std::forward<Subject>(subject));
  
  // 将 subject 和绑定值组合成一个 tuple
  auto full_args = std::tuple_cat(
      std::forward_as_tuple(std::forward<Subject>(subject)),
      std::move(bound_values));
  
  // 使用 std::apply 调用 handler
  return std::apply(c.handler, std::move(full_args));
}
```

关键点：
1. `pattern.bind(subject)` 返回绑定值的 tuple
2. `std::tuple_cat` 将 subject 和绑定值组合
3. `std::apply` 将组合后的参数传给 handler

### 3.3 pattern.bind() 的组合

`bind()` 方法是每个 pattern 必须实现的接口：

```cpp
// literal_pattern - 不绑定任何值
template <typename X>
constexpr auto bind(const X & /*subj*/) const {
  return std::tuple<>{};
}

// is_pattern - 绑定整个值
template <typename X>
constexpr auto bind(X &&x) const noexcept {
  return std::forward_as_tuple(std::forward<X>(x));
}
```

## 4. 类型检查机制

### 4.1 is_handler_invocable 的实现

**C++20 Concept 版本：**
```cpp
template <typename Case, typename Subject>
concept handler_invocable_for = requires(Case &&c, Subject &&s) {
  std::apply(
      std::forward<Case>(c).handler,
      std::tuple_cat(
          std::forward_as_tuple(std::forward<Subject>(s)),
          std::forward<Case>(c).pattern.bind(std::forward<Subject>(s))));
};
```

**C++17 SFINAE 版本：**
```cpp
template <typename Case, typename Subject>
struct is_handler_invocable {
private:
  using handler_type = case_handler_t<Case>;
  using pattern_type = case_pattern_t<Case>;
  using bound_args_tuple = pat::base::binding_args_t<pattern_type, Subject>;
  
  using full_invoke_args_tuple = decltype(std::tuple_cat(
      std::declval<std::tuple<const Subject &>>(),
      std::declval<bound_args_tuple>()));
      
public:
  static constexpr bool value = decltype(detail::is_applicable_impl<handler_type, full_invoke_args_tuple>(nullptr))::value;
};
```

### 4.2 为什么匹配 void handler 会失败

void handler 失败的原因：
1. `std::common_type_t` 无法处理 `void` 类型
2. 编译时需要确定所有 handler 的公共返回类型
3. `void` 不是有效的类型参数

### 4.3 为什么类型不一致会失败

类型不一致会失败的原因：
```cpp
template <typename Subject, typename Otherwise, typename... Cases>
struct match_result {
  using type = std::common_type_t<
      std::invoke_result_t<typename Cases::handler_type, Subject &>...,
      std::invoke_result_t<Otherwise, Subject &>>;
};
```

`std::common_type_t` 要求所有类型能转换为公共类型，如果 handler 返回不同类型（如 `int` 和 `string`），则无法确定公共类型。

### 4.4 match_result_t 和统一返回类型

所有 handler 必须统一返回类型的原因：
1. C++ 是强类型语言，表达式必须有确定的类型
2. `match` 表达式需要在编译时确定返回类型
3. `std::common_type_t` 用于计算公共返回类型

### 4.5 为什么 expression match 必须有公共返回类型

expression match（如 `match(value)` 作为表达式使用）必须有公共返回类型，因为：
1. 表达式类型必须在编译时确定
2. 返回值可以赋值给变量或用于其他表达式
3. 类型系统要求所有可能的返回路径类型一致

## 5. Pattern 的通用接口

### 5.1 pattern_like concept

**C++20 Concept：**
```cpp
template <typename P>
concept pattern = 
    std::derived_from<P, pattern_tag> || 
    requires(const P &p, auto &&subj) {
      { p.match(subj) } -> std::convertible_to<bool>;
    };
```

**C++17 Trait：**
```cpp
template <typename P>
struct is_pattern : std::integral_constant<
    bool,
    std::is_base_of_v<pattern_tag, P> ||
    ptn::pat::detail::has_match_method<P>::value> {};
```

### 5.2 match/bind 接口定义

所有 pattern 必须实现两个核心接口：

```cpp
// 匹配接口：返回 bool 表示是否匹配
template <typename Subject>
constexpr bool match(Subject &&subj) const;

// 绑定接口：返回要传递给 handler 的参数 tuple
template <typename Subject>
constexpr auto bind(Subject &&subj) const;
```

### 5.3 CRTP 基类

```cpp
template <typename Derived>
struct pattern_base : pattern_tag {
  static constexpr pattern_kind kind = pattern_kind::unknown;
  
  template <typename Subject>
  constexpr bool match(Subject &&subj) const 
      noexcept(noexcept(static_cast<const Derived *>(this)->match(
          std::forward<Subject>(subj)))) {
    return static_cast<const Derived *>(this)->match(std::forward<Subject>(subj));
  }
};
```

## 6. 所有 Pattern 的实现原理

### 6.1 Literal Pattern

```cpp
template <typename V, typename Cmp = std::equal_to<>>
struct literal_pattern : base::pattern_base<literal_pattern<V, Cmp>> {
  store_t v;  // 存储的值
  Cmp cmp;    // 比较器
  
  template <typename X>
  constexpr bool match(X const &x) const noexcept(noexcept(cmp(x, v))) {
    return cmp(x, v);
  }
  
  template <typename X>
  constexpr auto bind(const X & /*subj*/) const {
    return std::tuple<>{};  // 不绑定任何值
  }
};

// binding_args 特化
template <typename V, typename Cmp, typename Subject>
struct binding_args<ptn::pat::value::detail::literal_pattern<V, Cmp>, Subject> {
  using type = std::tuple<>;  // 声明不绑定参数
};
```

### 6.2 Type Pattern (is<T>)

```cpp
template <typename T>
struct is_pattern : ptn::pat::base::pattern_base<is_pattern<T>> {
  template <typename X>
  constexpr bool match(X &&) const noexcept {
    using D = ptn::meta::remove_cvref_t<X>;
    return std::is_same_v<D, T>;
  }
  
  template <typename X>
  constexpr auto bind(X &&x) const noexcept {
    return std::forward_as_tuple(std::forward<X>(x>));  // 绑定整个值
  }
};

// binding_args 特化
template <typename T, typename Subject>
struct binding_args<ptn::pat::type::detail::is_pattern<T>, Subject> {
  using type = std::tuple<Subject>;  // 声明绑定 Subject
};
```

### 6.3 Predicate Pattern

```cpp
template <typename F>
struct predicate_pattern : base::pattern_base<predicate_pattern<F>> {
  F fn;
  
  template <typename X>
  constexpr bool match(X const &x) const noexcept(noexcept(fn(x))) {
    return fn(x);
  }
  
  template <typename X>
  constexpr auto bind(const X & /*subj*/) const {
    return std::tuple<>{};  // 不绑定任何值
  }
};
```

### 6.4 逻辑组合 Pattern

```cpp
// AND Pattern
template <typename L, typename R>
struct and_pattern : base::pattern_base<and_pattern<L, R>> {
  L l; R r;
  
  template <typename X>
  constexpr bool match(X const &x) const noexcept(noexcept(l.match(x)) && noexcept(r.match(x))) {
    return l.match(x) && r.match(x);
  }
  
  template <typename X>
  constexpr auto bind(const X & /*subj*/) const {
    return std::tuple<>{};  // 逻辑组合不绑定值
  }
};
```

### 6.5 binding_args 特化的重要性

`binding_args` 特化是编译时类型推导的关键：

1. **编译时验证**：确保 `bind()` 方法的返回类型与声明一致
2. **类型推导**：为 `is_handler_invocable` 提供绑定参数类型信息
3. **接口契约**：明确定义每个 pattern 的绑定行为

## 7. DSL 运算符机制

### 7.1 >> 运算符

```cpp
template <typename Pattern, typename Handler>
constexpr auto operator>>(Pattern &&pattern, Handler &&handler) {
  using P = std::decay_t<Pattern>;
  using H = std::decay_t<Handler>;
  
  if constexpr (ptn::core::common::detail::is_value_like_v<H>) {
    // Pattern >> Value 语法糖
    auto value_handler = [val = std::forward<Handler>(handler)](auto &&...) -> H { 
      return val; 
    };
    return core::dsl::detail::case_expr<P, decltype(value_handler)>{
        std::forward<Pattern>(pattern), std::move(value_handler)};
  } else {
    // Pattern >> Handler
    return core::dsl::detail::case_expr<P, H>{
        std::forward<Pattern>(pattern), std::forward<Handler>(handler)};
  }
}
```

### 7.2 逻辑运算符

```cpp
// AND: p1 && p2
template <typename L, typename R>
constexpr auto operator&&(L &&l, R &&r) {
  return ptn::pat::value::detail::and_pattern<std::decay_t<L>, std::decay_t<R>>(
      std::forward<L>(l), std::forward<R>(r));
}

// OR: p1 || p2
template <typename L, typename R>
constexpr auto operator||(L &&l, R &&r) {
  return ptn::pat::value::detail::or_pattern<std::decay_t<L>, std::decay_t<R>>(
      std::forward<L>(l), std::forward<R>(r));
}

// NOT: !p
template <typename P>
constexpr auto operator!(P &&p) {
  return ptn::pat::value::detail::not_pattern<std::decay_t<P>>(
      std::forward<P>(p));
}
```

## 8. 编译时诊断系统

### 8.1 静态断言层级

```cpp
// 1. 验证整个 match 表达式
template <typename Subject, typename Otherwise, typename... Cases>
constexpr void static_assert_valid_match() {
  static_assert((is_handler_invocable_v<Cases, Subject> && ...),
                "At least one case's handler cannot be invoked");
  static_assert(std::is_invocable_v<Otherwise, Subject> || std::is_invocable_v<Otherwise>,
                "The `otherwise` handler has an invalid signature");
  using common_return_type = match_result_t<Subject, Otherwise, Cases...>;
  (void) sizeof(common_return_type);  // 强制计算公共返回类型
}

// 2. 验证单个 case
template <typename Case, typename Subject>
constexpr void static_assert_valid_case() {
  static_assert(is_case_expr_v<Case>,
                "Argument to `.when()` must be a case expression");
  static_assert_valid_handler<Case, Subject>();
}

// 3. 验证 handler
template <typename Case, typename Subject>
constexpr void static_assert_valid_handler() {
  static_assert(is_handler_invocable_v<Case, Subject>,
                "Handler cannot be invoked with the arguments bound by the pattern");
}
```