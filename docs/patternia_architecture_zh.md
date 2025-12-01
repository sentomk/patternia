# Patternia 模式匹配库架构分析

## 第一部分：命名空间结构

Patternia 库采用了清晰的层次化命名空间结构，主要分为以下几个层次：

### 根命名空间 `ptn`
Patternia 库的根命名空间，所有公共API都位于此命名空间下。

### 核心子命名空间

#### `ptn::core`
核心引擎和DSL相关功能
- `ptn::core::common` - 通用工具和诊断
- `ptn::core::dsl` - 领域特定语言操作符
- `ptn::core::engine` - 匹配引擎核心实现

#### `ptn::pat`
模式定义和实现
- `ptn::pat::base` - 基础模式抽象和特征
- `ptn::pat::value` - 值模式（字面量、关系、谓词）
- `ptn::pat::type` - 类型模式

#### `ptn::meta`
元编程工具
- `ptn::meta::base` - 基础类型特征
- `ptn::meta::dsa` - 数据结构和算法
- `ptn::meta::query` - 类型查询工具

### 实现细节命名空间
- `ptn::core::engine::detail` - 引擎实现细节
- `ptn::core::dsl::detail` - DSL实现细节
- `ptn::pat::value::detail` - 值模式实现细节
- `ptn::pat::type::detail` - 类型模式实现细节

## 第二部分：层次划分

Patternia 库采用了经典的分层架构设计，从底层到顶层分为以下层次：

### Meta Layer (元编程层)
**位置**: `ptn::meta/`
**职责**: 提供编译时类型操作和元编程基础设施
- `base/traits.hpp` - 基础类型特征（remove_cvref_t, is_spec_of等）
- `dsa/` - 数据结构和算法（type_list, algorithms）
- `query/` - 类型查询工具（index, template_info）

### Core Layer (核心层)
**位置**: `ptn/core/`
**职责**: 提供模式匹配的核心引擎和DSL支持

#### Common Sublayer
- `common/` - 通用工具、诊断、优化、评估

#### DSL Sublayer  
- `dsl/` - 领域特定语言操作符（>>, &&, ||, !）

#### Engine Sublayer
- `engine/` - 匹配引擎实现（builder, match）

### Pattern Layer (模式层)
**位置**: `ptn/pattern/`
**职责**: 定义各种类型的模式和模式工厂

#### Base Sublayer
- `base/` - 模式基类、特征、绑定契约

#### Value Pattern Sublayer
- `value/` - 值模式（字面量、关系比较、谓词）

#### Type Pattern Sublayer  
- `type/` - 类型模式（is, in, not_in, from）

### Configuration Layer (配置层)
**位置**: `ptn/config.hpp`
**职责**: 编译时配置开关和特性控制

### Integration Layer (集成层)
**位置**: `ptn/patternia.hpp`
**职责**: 统一的公共入口点，导出所有必要的头文件

## 第三部分：API汇总及其语义

### 核心匹配API

#### `ptn::match(T&& value)`
**语义**: 模式匹配的入口点，创建匹配构建器
**参数**: 要匹配的值
**返回**: `match_builder` 对象，支持链式调用

#### `ptn::match<U>(T&& value)`
**语义**: 显式指定类型的模式匹配入口点
**参数**: 目标类型U和要转换的值
**返回**: 指定类型的 `match_builder` 对象

### 模式构建器API

#### `when(case_expr)`
**语义**: 添加一个匹配条件和对应的处理函数
**参数**: 使用 `>>` 操作符创建的case表达式
**返回**: 新的匹配构建器（支持链式调用）

#### `otherwise(handler)`
**语义**: 设置默认处理函数，当所有模式都不匹配时执行
**参数**: 处理函数或返回值
**返回**: 匹配结果

### DSL操作符API

#### `pattern >> handler`
**语义**: 将模式与处理函数关联
**特殊**: 如果handler是值类型，自动转换为返回该值的函数

#### `pattern1 && pattern2`
**语义**: 逻辑AND组合，两个模式都必须匹配

#### `pattern1 || pattern2`
**语义**: 逻辑OR组合，任一模式匹配即可

#### `!pattern`
**语义**: 逻辑NOT，模式不匹配时成功

### 值模式API

#### 字面量模式
- `lit(value)` - 精确匹配字面值
- `lit_ci(value)` - 大小写不敏感的字符串匹配

#### 关系模式
- `lt(value)` - 小于 (x < value)
- `le(value)` - 小于等于 (x <= value)
- `gt(value)` - 大于 (x > value)
- `ge(value)` - 大于等于 (x >= value)
- `eq(value)` - 等于 (x == value)
- `ne(value)` - 不等于 (x != value)
- `between(lo, hi, closed=true)` - 区间匹配

#### 谓词模式
- `pred(predicate)` - 使用任意谓词函数

### 类型模式API

#### `ptn::pat::type::is<T>`
**语义**: 精确类型匹配，匹配成功时绑定值到处理函数
**绑定**: 绑定匹配的值作为 `std::tuple<Subject>`

#### `ptn::pat::type::in<Types...>`
**语义**: 类型集合匹配，检查类型是否在给定的类型列表中

#### `ptn::pat::type::not_in<Types...>`
**语义**: 类型排除匹配，检查类型不在给定的类型列表中

#### `ptn::pat::type::from<T>`
**语义**: 类型转换匹配，检查是否可以从目标类型转换而来

### 元编程API

#### 类型特征
- `ptn::meta::remove_cvref_t<T>` - 移除const/volatile和引用
- `ptn::meta::is_spec_of_v<Template, T>` - 检查是否为模板特化
- `ptn::meta::is_tmpl_v<T>` - 检查是否为模板实例

#### 类型列表操作
- `ptn::meta::dsa::type_list` - 类型列表容器
- `ptn::meta::dsa::algorithms` - 类型列表算法

### 配置宏
- `PTN_ENABLE_VALUE_PATTERN` - 启用值模式
- `PTN_ENABLE_RELATIONAL_PATTERN` - 启用关系模式
- `PTN_ENABLE_TYPE_PATTERN` - 启用类型模式
- `PTN_ENABLE_PREDICATE_PATTERN` - 启用谓词模式
- `PTN_USE_CONCEPTS` - 启用C++20概念支持

### 绑定语义
- **字面量模式**: 不绑定任何值（`std::tuple<>`）
- **关系模式**: 不绑定任何值（`std::tuple<>`）
- **谓词模式**: 不绑定任何值（`std::tuple<>`）
- **类型模式**: 绑定匹配的值（`std::tuple<Subject>`）

### 错误处理
库使用 `static_assert` 在编译时验证：
- 模式类型的有效性
- 处理函数签名的兼容性
- 类型的可构造性
- 操作符的支持性

这个设计确保了类型安全和高性能，同时提供了直观易用的DSL语法。
