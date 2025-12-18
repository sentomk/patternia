# From Control Flow to Pattern Matching

> Start here if you are new to pattern matching.

This tutorial guides you through the journey from traditional control flow structures to the elegant world of pattern matching. You'll learn how pattern matching can make your code more readable, maintainable, and expressive.

---

## 1. The Problem with Traditional Control Flow

### Traditional if-else Chains

Consider this common scenario:

```cpp
#include <iostream>
#include <variant>
#include <string>

struct HttpRequest { std::string method; std::string path; };
struct HttpResponse { int status; std::string body; };
struct WebSocketMessage { std::string type; std::string payload; };

using Message = std::variant<HttpRequest, HttpResponse, WebSocketMessage>;

void processMessage(const Message& msg) {
    if (auto* http_req = std::get_if<HttpRequest>(&msg)) {
        if (http_req->method == "GET") {
            if (http_req->path == "/api/users") {
                std::cout << "Get all users\n";
            } else if (http_req->path == "/api/posts") {
                std::cout << "Get all posts\n";
            } else {
                std::cout << "Unknown GET endpoint\n";
            }
        } else if (http_req->method == "POST") {
            if (http_req->path == "/api/users") {
                std::cout << "Create user\n";
            } else {
                std::cout << "Unknown POST endpoint\n";
            }
        } else {
            std::cout << "Unsupported HTTP method\n";
        }
    } else if (auto* http_resp = std::get_if<HttpResponse>(&msg)) {
        if (http_resp->status == 200) {
            std::cout << "Success: " << http_resp->body << "\n";
        } else if (http_resp->status == 404) {
            std::cout << "Not found\n";
        } else {
            std::cout << "HTTP status: " << http_resp->status << "\n";
        }
    } else if (auto* ws_msg = std::get_if<WebSocketMessage>(&msg)) {
        if (ws_msg->type == "chat") {
            std::cout << "Chat message: " << ws_msg->payload << "\n";
        } else if (ws_msg->type == "ping") {
            std::cout << "Ping received\n";
        } else {
            std::cout << "Unknown WebSocket message type\n";
        }
    }
}
```

**Problems with this approach:**
- Deeply nested conditionals are hard to read
- Error-prone due to manual type checking
- Difficult to maintain as conditions grow
- No compile-time guarantee of completeness
- Business logic gets lost in control flow noise

### Traditional switch Statements

Switch statements are better for simple cases, but still limited:

```cpp
enum class Color { Red, Green, Blue, Yellow, Purple };

std::string colorToString(Color color) {
    switch (color) {
        case Color::Red:
            return "Red";
        case Color::Green:
            return "Green";
        case Color::Blue:
            return "Blue";
        case Color::Yellow:
            return "Yellow";
        case Color::Purple:
            return "Purple";
        default:
            return "Unknown";  // This should never happen!
    }
}
```

**Limitations:**
- Only works on integral types
- Can't match complex data structures
- No built-in way to extract values
- Falls back to `default` instead of ensuring completeness

---

## 2. Enter Pattern Matching

Pattern matching treats control flow as **data shape matching** rather than value comparison. Let's see how Patternia transforms these examples.

### Deconstructing Complex Conditions

The nested `if-else` chain becomes a flat, readable structure:

```cpp
#include <ptn/patternia.hpp>
using namespace ptn;

struct HttpRequest { std::string method; std::string path; };
struct HttpResponse { int status; std::string body; };
struct WebSocketMessage { std::string type; std::string payload; };

using Message = std::variant<HttpRequest, HttpResponse, WebSocketMessage>;

void processMessage(const Message& msg) {
    match(msg)
        // HTTP Request cases
        .when(has<HttpRequest>()[bind(&HttpRequest::method) == "GET" && 
                                 bind(&HttpRequest::path) == "/api/users"] >>
              []() { std::cout << "Get all users\n"; })
        .when(has<HttpRequest>()[bind(&HttpRequest::method) == "GET" && 
                                 bind(&HttpRequest::path) == "/api/posts"] >>
              []() { std::cout << "Get all posts\n"; })
        .when(has<HttpRequest>()[bind(&HttpRequest::method) == "POST" && 
                                 bind(&HttpRequest::path) == "/api/users"] >>
              []() { std::cout << "Create user\n"; })
        
        // HTTP Response cases
        .when(has<HttpResponse>()[bind(&HttpResponse::status) == 200] >>
              [](auto resp) { std::cout << "Success: " << resp.body << "\n"; })
        .when(has<HttpResponse>()[bind(&HttpResponse::status) == 404] >>
              [](auto) { std::cout << "Not found\n"; })
        
        // WebSocket cases
        .when(has<WebSocketMessage>()[bind(&WebSocketMessage::type) == "chat"] >>
              [](auto ws) { std::cout << "Chat: " << ws.payload << "\n"; })
        .when(has<WebSocketMessage>()[bind(&WebSocketMessage::type) == "ping"] >>
              [](auto) { std::cout << "Ping received\n"; })
        
        .otherwise([]() { std::cout << "Unhandled message\n"; });
}
```

**Benefits:**
- **Flat structure**: No deep nesting
- **Declarative**: Each case describes what to match
- **Type-safe**: Compile-time checking of patterns
- **Extensible**: Easy to add new cases

### Exhaustive Matching for Enums

For enums, we can ensure completeness:

```cpp
#include <ptn/patternia.hpp>
using namespace ptn;

enum class Color { Red, Green, Blue, Yellow, Purple };

std::string colorToString(Color color) {
    return match(color)
        .when(lit(Color::Red) >> std::string("Red"))
        .when(lit(Color::Green) >> std::string("Green"))
        .when(lit(Color::Blue) >> std::string("Blue"))
        .when(lit(Color::Yellow) >> std::string("Yellow"))
        .when(lit(Color::Purple) >> std::string("Purple"))
        .end();  // Compiler ensures all cases are covered
}
```

**Advantages:**
- **Compile-time completeness**: No forgotten cases
- **No default needed**: All cases explicitly handled
- **Type safety**: Can't miss enum values

---

## 3. Pattern Matching Concepts

### 3.1 Patterns vs Values

Traditional control flow compares **values**:

```cpp
if (x == 5) { /* ... */ }
if (str == "hello") { /* ... */ }
```

Pattern matching matches **patterns**:

```cpp
match(x)
    .when(lit(5) >> [] { /* ... */ })           // Literal pattern
    .when(bind()[_ > 0 && _ < 10] >> [] { /* ... */ })  // Guard pattern
    .when(has<std::pair<int, int>>() >> [] { /* ... */ }) // Type pattern
```

### 3.2 Pattern Types

#### Literal Patterns
```cpp
.when(lit(42) >> [] { /* matches exactly 42 */ })
.when(lit("hello") >> [] { /* matches exactly "hello" */ })
```

#### Wildcard Pattern
```cpp
.when(__ >> [] { /* matches anything */ })
```

#### Binding Patterns
```cpp
.when(bind() >> [](int value) { /* value is the matched value */ })
.when(bind(&Point::x) >> [](int x) { /* x is the x coordinate */ })
```

#### Type Patterns
```cpp
.when(has<std::string>() >> [](const std::string& s) { /* matched string */ })
.when(has<std::pair<int, int>>() >> [](auto pair) { /* matched pair */ })
```

#### Structural Patterns
```cpp
.when(has<Point>(lit(0), bind()) >> [](int y) { /* x=0, bind y */ })
```

---

## 4. From Imperative to Declarative

### Imperative Style (Traditional)

```cpp
// Find first positive even number
int findFirstPositiveEven(const std::vector<int>& nums) {
    for (size_t i = 0; i < nums.size(); ++i) {
        if (nums[i] > 0 && nums[i] % 2 == 0) {
            return nums[i];
        }
    }
    return -1;  // Not found
}
```

### Declarative Style (Pattern Matching)

```cpp
int findFirstPositiveEven(const std::vector<int>& nums) {
    return match(nums)
        .when(has<std::vector<int>>()[/* find first matching element */] >> 
              [](int first_even) { return first_even; })
        .otherwise(-1);
}
```

*Note: This example shows the conceptual difference. In practice, you might combine pattern matching with algorithms for the best of both worlds.*

---

## 5. Real-World Migration Examples

### Example 1: Network Protocol Handler

#### Before (Traditional)
```cpp
void handlePacket(const std::vector<uint8_t>& packet) {
    if (packet.size() < 4) {
        std::cout << "Invalid packet\n";
        return;
    }
    
    uint16_t type = (packet[0] << 8) | packet[1];
    uint16_t length = (packet[2] << 8) | packet[3];
    
    if (type == 0x0001) {  // Login packet
        if (packet.size() != 4 + 32) {
            std::cout << "Invalid login packet\n";
            return;
        }
        std::string username(packet.begin() + 4, packet.begin() + 20);
        std::string password(packet.begin() + 20, packet.begin() + 36);
        handleLogin(username, password);
    } else if (type == 0x0002) {  // Data packet
        if (packet.size() != 4 + length) {
            std::cout << "Invalid data packet\n";
            return;
        }
        std::vector<uint8_t> data(packet.begin() + 4, packet.end());
        handleData(data);
    } else if (type == 0x0003) {  // Heartbeat
        if (packet.size() != 4) {
            std::cout << "Invalid heartbeat\n";
            return;
        }
        handleHeartbeat();
    } else {
        std::cout << "Unknown packet type\n";
    }
}
```

#### After (Pattern Matching)
```cpp
struct LoginPacket { std::string username; std::string password; };
struct DataPacket { std::vector<uint8_t> data; };
struct HeartbeatPacket {};

using Packet = std::variant<LoginPacket, DataPacket, HeartbeatPacket>;

Packet parsePacket(const std::vector<uint8_t>& raw) {
    // Implementation would parse raw bytes into variant
    // ...
}

void handlePacket(const std::vector<uint8_t>& packet) {
    match(parsePacket(packet))
        .when(has<LoginPacket>() >> 
              [](const LoginPacket& login) { handleLogin(login.username, login.password); })
        .when(has<DataPacket>() >> 
              [](const DataPacket& data) { handleData(data.data); })
        .when(has<HeartbeatPacket>() >> 
              [](const HeartbeatPacket&) { handleHeartbeat(); })
        .otherwise([]() { std::cout << "Invalid packet\n"; });
}
```

### Example 2: Configuration Processing

#### Before (Traditional)
```cpp
#include <nlohmann/json.hpp>

void processConfig(const nlohmann::json& config) {
    if (!config.contains("type")) {
        throw std::runtime_error("Missing type field");
    }
    
    std::string type = config["type"];
    
    if (type == "database") {
        if (!config.contains("host") || !config.contains("port")) {
            throw std::runtime_error("Missing database config");
        }
        std::string host = config["host"];
        int port = config["port"];
        std::string database = config.value("database", "default");
        setupDatabase(host, port, database);
    } else if (type == "file") {
        if (!config.contains("path")) {
            throw std::runtime_error("Missing file path");
        }
        std::string path = config["path"];
        bool readonly = config.value("readonly", false);
        setupFileStorage(path, readonly);
    } else if (type == "memory") {
        int max_size = config.value("max_size", 1024);
        setupMemoryStorage(max_size);
    } else {
        throw std::runtime_error("Unknown config type: " + type);
    }
}
```

#### After (Pattern Matching)
```cpp
struct DatabaseConfig { std::string host; int port; std::string database = "default"; };
struct FileConfig { std::string path; bool readonly = false; };
struct MemoryConfig { int max_size = 1024; };

using Config = std::variant<DatabaseConfig, FileConfig, MemoryConfig>;

void processConfig(const Config& config) {
    match(config)
        .when(has<DatabaseConfig>() >> 
              [](const DatabaseConfig& db) { 
                  setupDatabase(db.host, db.port, db.database); 
              })
        .when(has<FileConfig>() >> 
              [](const FileConfig& file) { 
                  setupFileStorage(file.path, file.readonly); 
              })
        .when(has<MemoryConfig>() >> 
              [](const MemoryConfig& mem) { 
                  setupMemoryStorage(mem.max_size); 
              });
}
```

---

## 6. When to Use Pattern Matching

### Ideal Scenarios

#### 1. Complex Data Validation
```cpp
match(user_input)
    .when(has<std::string>()[std::regex_match(_, std::regex("\\d{4}-\\d{2}-\\d{2}"))] >>
          [](const std::string& date) { processDate(date); })
    .when(has<std::string>()[std::regex_match(_, std::regex("\\d+"))] >>
          [](const std::string& number) { processNumber(std::stoi(number)); })
    .otherwise([]() { throw std::invalid_argument("Invalid input"); });
```

#### 2. State Machine Transitions
```cpp
enum class State { Idle, Connecting, Connected, Error };
enum class Event { Connect, Disconnect, Data, Timeout };

State transition(State current, Event event) {
    return match(current, event)
        .when(lit(State::Idle, Event::Connect) >> State::Connecting)
        .when(lit(State::Connecting, Event::Data) >> State::Connected)
        .when(lit(State::Connected, Event::Disconnect) >> State::Idle)
        .when(__, Event::Timeout) >> State::Error)  // Any state + timeout
        .otherwise(current);  // No transition
}
```

#### 3. AST Processing
```cpp
struct Number { double value; };
struct BinaryOp { std::string op; std::unique_ptr<Expr> left, right; };
struct Variable { std::string name; };

using Expr = std::variant<Number, BinaryOp, Variable>;

double evaluate(const Expr& expr, const std::map<std::string, double>& vars) {
    return match(expr)
        .when(has<Number>() >> [](const Number& n) { return n.value; })
        .when(has<Variable>() >> 
              [&vars](const Variable& v) { 
                  auto it = vars.find(v.name);
                  return it != vars.end() ? it->second : 0.0; 
              })
        .when(has<BinaryOp>() >> 
              [&](const BinaryOp& op) {
                  double left = evaluate(*op.left, vars);
                  double right = evaluate(*op.right, vars);
                  
                  return match(op.op)
                      .when(lit("+") >> left + right)
                      .when(lit("-") >> left - right)
                      .when(lit("*") >> left * right)
                      .when(lit("/") >> right != 0 ? left / right : 0.0)
                      .otherwise(0.0);
              });
}
```

### When Pattern Matching Might Not Be the Best Choice

#### 1. Simple Boolean Conditions
```cpp
// Overkill for pattern matching
if (user.age >= 18 && user.is_verified) {
    grant_access();
}

// Keep it simple
```

#### 2. Performance-Critical Inner Loops
```cpp
// Traditional approach might be faster for hot paths
for (int i = 0; i < million; ++i) {
    if (data[i].type == TARGET_TYPE) {
        process(data[i]);
    }
}
```

#### 3. When You Need Side Effects in Guards
```cpp
// Pattern matching guards should be pure
// Use traditional if-else for side-effecting conditions
```

---

## 7. Migration Strategy

### Step 1: Identify Candidates

Look for code with these characteristics:
- Deeply nested `if-else` chains
- Multiple type checks (`if (auto* ptr = get_if<...>)`)
- `switch` statements on enums
- Complex validation logic

### Step 2: Start Small

Choose a simple function first:
```cpp
// Before
std::string formatNumber(int n) {
    if (n == 0) return "zero";
    if (n > 0) return "positive: " + std::to_string(n);
    return "negative: " + std::to_string(n);
}

// After
std::string formatNumber(int n) {
    return match(n)
        .when(lit(0) >> std::string("zero"))
        .when(bind()[_ > 0] >> [](int n) { return "positive: " + std::to_string(n); })
        .otherwise([](int n) { return "negative: " + std::to_string(n); });
}
```

### Step 3: Gradual Refactoring

1. **Replace type discrimination** first
2. **Add pattern extraction** next
3. **Incorporate guards** last
4. **Eliminate default cases** for exhaustive matching

### Step 4: Establish Patterns

Create reusable patterns for common scenarios:

```cpp
// Common validation pattern
auto non_empty_string = has<std::string>()[!_.empty()];
auto positive_number = bind<int>()[_ > 0];

match(input)
    .when(non_empty_string >> [](const std::string& s) { process_string(s); })
    .when(positive_number >> [](int n) { process_number(n); })
    .otherwise([]() { throw std::invalid_argument("Invalid input"); });
```

---

## 8. Best Practices

### 1. Keep Patterns Simple
```cpp
// Good: Clear, focused patterns
.when(has<User>()[bind(&User::age) >= 18] >> [](const User& user) { /* ... */ })

// Avoid: Overly complex patterns
.when(has<ComplexData>()[bind(&ComplexData::field1) == val1 && 
                        bind(&ComplexData::field2)[subfield] > threshold &&
                        /* many more conditions */] >> /* ... */)
```

### 2. Use Meaningful Bindings
```cpp
// Good: Descriptive binding names
.when(has<Point>(bind(&Point::x), bind(&Point::y)) >> 
      [](int x_coord, int y_coord) { /* ... */ })

// Avoid: Generic names
.when(has<Point>(bind(), bind()) >> [](int a, int b) { /* ... */ })
```

### 3. Order Matters
```cpp
// Place more specific patterns first
match(value)
    .when(has<std::string>()[_.length() > 10] >> handle_long_string)
    .when(has<std::string>() >> handle_string)  // General case after specific
    .otherwise(handle_other);
```

### 4. Prefer Exhaustive Matching
```cpp
// Good: Compile-time completeness
enum class Status { Ok, Error, Pending };

match(status)
    .when(lit(Status::Ok) >> handle_ok)
    .when(lit(Status::Error) >> handle_error)
    .when(lit(Status::Pending) >> handle_pending)
    .end();  // Compiler warns if cases missing
```

---

## 9. Testing Pattern Matching Code

### Unit Test Structure

```cpp
#include <catch2/catch_test_macros.hpp>
#include <ptn/patternia.hpp>

TEST_CASE("Message processing", "[pattern]") {
    using namespace ptn;
    
    SECTION("HTTP GET request") {
        Message msg = HttpRequest{"GET", "/api/users"};
        std::string result = captureOutput([&]() { processMessage(msg); });
        REQUIRE(result == "Get all users\n");
    }
    
    SECTION("Unknown message type") {
        Message msg = WebSocketMessage{"unknown", "data"};
        std::string result = captureOutput([&]() { processMessage(msg); });
        REQUIRE(result == "Unhandled message\n");
    }
}
```

### Property-Based Testing

```cpp
// Test that all enum values are handled
TEST_CASE("All enum cases handled", "[exhaustive]") {
    for (auto color : {Color::Red, Color::Green, Color::Blue, Color::Yellow, Color::Purple}) {
        REQUIRE_NOTHROW([&]() { 
            colorToString(color); 
        }());
    }
}
```

---

## 10. Conclusion

Pattern matching transforms how we think about control flow:

| Traditional Control Flow | Pattern Matching |
|--------------------------|------------------|
| Imperative ("how to check") | Declarative ("what to match") |
| Value comparison | Pattern matching |
| Manual error handling | Compile-time safety |
| Deep nesting | Flat structure |
| Ad-hoc validation | Structured patterns |

### Key Takeaways

1. **Start with complex, nested conditionals** - they benefit most from pattern matching
2. **Think in patterns, not values** - focus on the shape and structure of data
3. **Use exhaustive matching** when possible - let the compiler help you
4. **Keep patterns readable** - complex logic belongs in handlers, not patterns
5. **Migrate gradually** - pattern matching works alongside traditional code

### Next Steps

- Practice with simple examples in your codebase
- Explore the [API Documentation](../api.md) for advanced patterns
- Read the [Getting Started Guide](../guide/getting-started.md) for comprehensive examples
- Check out the [samples directory](../../samples/) for real-world usage

Welcome to the world of pattern matching - where control flow becomes data flow!
