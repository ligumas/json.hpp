<div align="center">

# json.hpp

**Single-header JSON parser for C++17**

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square)
![Header only](https://img.shields.io/badge/header--only-yes-brightgreen?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-blue?style=flat-square)

</div>

---

Drop `json.hpp` into your project. Parse JSON. Done.

```cpp
#include "json.hpp"

auto v = json::parse(R"({
    "name": "jasper",
    "age": 20,
    "tags": ["cpp", "linux"]
})");

std::string name = v["name"].as_string();   // "jasper"
int age          = v["age"].as_int();        // 20
std::string tag  = v["tags"][0].as_string(); // "cpp"
```

---

## Install

Copy `json.hpp` to your project. That's it.

---

## API

### Parsing

```cpp
json::Value v = json::parse(str);   // throws json::ParseError on bad input
```

### Type checks

```cpp
v.is_null()
v.is_bool()
v.is_number()
v.is_string()
v.is_array()
v.is_object()
```

### Reading values

```cpp
bool   b = v.as_bool();
double n = v.as_number();
int    i = v.as_int();
const std::string& s = v.as_string();
```

### Objects and arrays

```cpp
v["key"]    // object access (throws std::out_of_range if missing)
v[0]        // array access
v.size()    // element count
v.contains("key")  // check key exists without throwing
```

### Serializing

```cpp
std::string s = json::dump(v);       // pretty-printed (2 space indent)
std::string s = json::dump(v, 4);    // 4 space indent
std::string s = json::dump(v, 0);    // compact
```

### Building values

```cpp
json::Value obj = json::Object{};
obj.as_object()["x"] = json::Value(42);
obj.as_object()["name"] = json::Value("hello");

json::Value arr = json::Array{ json::Value(1), json::Value(2), json::Value(3) };
```

### Errors

```cpp
try {
    auto v = json::parse("{ bad json }");
} catch (const json::ParseError& e) {
    std::cerr << e.what(); // includes line and col
}
```

---

## What it handles

- Objects, arrays, strings, numbers (int + float), booleans, null
- Nested structures
- Escape sequences (`\n`, `\t`, `\\`, `\"`, etc.)
- Decent error messages with line/column info

## What it doesn't

- Unicode (`\uXXXX`) — skipped, not decoded
- Streaming / incremental parsing
- Comments (not valid JSON anyway)

---

## License

MIT
