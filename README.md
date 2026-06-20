<div align="center">

# json.hpp

single-header JSON parser for C++17

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square)
![header-only](https://img.shields.io/badge/header--only-yes-brightgreen?style=flat-square)
![license](https://img.shields.io/badge/license-MIT-blue?style=flat-square)

</div>

drop `json.hpp` into your project. parse JSON. done.

```cpp
#include "json.hpp"

auto v = json::parse(R"({
    "name": "jasper",
    "age": 20,
    "tags": ["cpp", "linux"]
})");

std::string name = v["name"].as_string();    // "jasper"
int age          = v["age"].as_int();         // 20
std::string tag  = v["tags"][0].as_string();  // "cpp"
```

## install

copy `json.hpp` to your project.

## API

```cpp
json::Value v = json::parse(str);   // throws json::ParseError on bad input

v.is_null() / is_bool() / is_number() / is_string() / is_array() / is_object()

v.as_bool() / as_number() / as_int() / as_string()
v["key"]        // object access — throws if key missing
v[0]            // array access
v.size()
v.empty()
v.contains("key")

// typed access with default — returns def if key missing or wrong type
int   n = v.get<int>("count", 0);
bool  b = v.get<bool>("flag", false);
auto  s = v.get<std::string>("name", "");
float f = v.get<float>("ratio", 0.0f);

// safe access: returns pointer to value, or nullptr if key is missing
if (const json::Value* p = v.try_get("optional_field")) {
    use(p->as_string());
}

// access with fallback Value
json::Value fallback = 42;
json::Value x = v.value("count", fallback);   // returns fallback if key missing

std::string s = json::dump(v);       // pretty-printed, 2 space indent
std::string s = json::dump(v, 0);    // compact
```

building values:

```cpp
auto obj = json::object({
    {"name", "jasper"},
    {"age", 20},
    {"tags", json::array({"cpp", "linux"})}
});

auto arr = json::array({1, 2, 3});

std::string s = json::dump(obj);  // serialize back to JSON string
```

errors include line and column:

```cpp
try {
    auto v = json::parse("{ bad }");
} catch (const json::ParseError& e) {
    std::cerr << e.what();  // "unexpected character 'b' (line 1, col 3)"
}
```

handles objects, arrays, strings, numbers (int/float), bools, null, and escape sequences including `\uXXXX` (decoded to UTF-8, surrogate pairs supported).

**License:** MIT
