<br><br>

<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <source media="(prefers-color-scheme: light)" srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <img alt="Patternia logo"
         width="420"
         style="max-width: 90%; height: auto; margin-top: 10px; transform: translateX(-2px);">
  </picture>
</div>

<br><br>

[![C++17+](https://img.shields.io/badge/C%2B%2B-17%2B-blue.svg?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Build](https://github.com/SentoMK/patternia/actions/workflows/ci.yml/badge.svg?branch=main&style=flat-square)](https://github.com/SentoMK/patternia/actions)
[![License](https://img.shields.io/github/license/SentoMK/patternia?style=flat-square)](LICENSE)
[![Version](https://img.shields.io/github/v/release/SentoMK/patternia?style=flat-square&color=orange)](https://github.com/SentoMK/patternia/releases)
[![Docs](https://img.shields.io/badge/docs-online-blue?style=flat-square)](https://sentomk.github.io/patternia)

<br>

**A header-only, zero-overhead, compile-time pattern matching library for modern C++.**

---

### Examples

> ⚠️ From v0.4.0+, `.with()` is considered **internal API**.
>
> Always prefer `.when(pattern >> handler)` in user code.

#### 🛠️ Optional: Recommended Code Style

To make your chained `.when()` and `.otherwise()` expressions stay perfectly aligned, add this minimal `.clang-format` to your project root:

```yaml
# patternia .clang-format (minimal)
BasedOnStyle: LLVM
IndentWidth: 2 # or 4
ContinuationIndentWidth: 4 # or 6
ColumnLimit: 0
BinPackArguments: false
BinPackParameters: false
BreakBeforeBinaryOperators: None
```

✅ Resulting style:

```cpp
auto out =
    match(5)
      .when(lt(0) >> "neg")
      .when(gt(0) >> "pos")
      .otherwise("other");
```
