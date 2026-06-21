#include "json.hpp"
#include <cassert>
#include <iostream>
#include <cstring>

int ok = 0, fail = 0;

#define check(cond, msg) \
    if (cond) { ok++; } \
    else { std::cerr << "FAIL: " << msg << "\n"; fail++; }

void test_basic() {
    auto v = json::parse(R"({"name":"jasper","age":20,"active":true})");
    check(v.is_object(), "is object");
    check(v["name"].as_string() == "jasper", "string value");
    check(v["age"].as_int() == 20, "number value");
    check(v["active"].as_bool() == true, "bool value");
}

void test_array() {
    auto v = json::parse("[1, 2, 3, 4]");
    check(v.is_array(), "is array");
    check(v.size() == 4, "array size");
    check(v[0].as_int() == 1, "array[0]");
    check(v[3].as_int() == 4, "array[3]");
}

void test_nested() {
    auto v = json::parse(R"({
        "user": {
            "id": 42,
            "tags": ["cpp", "linux", "systems"]
        }
    })");
    check(v["user"]["id"].as_int() == 42, "nested int");
    check(v["user"]["tags"][1].as_string() == "linux", "nested array string");
}

void test_null() {
    auto v = json::parse("null");
    check(v.is_null(), "null value");
}

void test_empty() {
    auto v = json::parse("{}");
    check(v.is_object() && v.size() == 0, "empty object");
    auto a = json::parse("[]");
    check(a.is_array() && a.size() == 0, "empty array");
}

void test_escapes() {
    auto v = json::parse(R"({"msg":"hello\nworld\ttab"})");
    check(v["msg"].as_string() == "hello\nworld\ttab", "escape sequences");
}

void test_float() {
    auto v = json::parse("3.14");
    check(std::abs(v.as_number() - 3.14) < 0.001, "float");
}

void test_dump() {
    auto v = json::parse(R"({"x":1,"arr":[1,2]})");
    std::string s = json::dump(v);
    check(s.find("\"x\"") != std::string::npos, "dump contains key");
    check(s.find("1") != std::string::npos, "dump contains value");
}

void test_bool_false() {
    auto v = json::parse("false");
    check(v.is_bool() && v.as_bool() == false, "false literal");
}

void test_contains() {
    auto v = json::parse(R"({"a":1})");
    check(v.contains("a"), "contains existing key");
    check(!v.contains("b"), "contains missing key");
}

void test_get_typed() {
    auto v = json::parse(R"({"count":42,"ratio":3.14,"flag":true,"name":"jasper"})");
    check(v.get<int>("count", 0) == 42,        "get<int>");
    check(v.get<int>("missing", -1) == -1,     "get<int> default");
    check(std::abs(v.get<float>("ratio", 0.0f) - 3.14f) < 0.001f, "get<float>");
    check(v.get<bool>("flag", false) == true,  "get<bool>");
    check(v.get<std::string>("name", "") == "jasper", "get<string>");
    check(v.get<std::string>("missing", "def") == "def", "get<string> default");
}

void test_try_get() {
    auto v = json::parse(R"({"x":10})");
    const json::Value* p = v.try_get("x");
    check(p != nullptr && p->as_int() == 10, "try_get existing");
    const json::Value* q = v.try_get("y");
    check(q == nullptr, "try_get missing");
}

void test_value_fallback() {
    auto v = json::parse(R"({"a":1})");
    json::Value fallback = 99;
    json::Value got = v.value("a", fallback);
    check(got.as_int() == 1, "value existing");
    json::Value missing = v.value("z", fallback);
    check(missing.as_int() == 99, "value fallback");
}

void test_builders() {
    auto obj = json::object({
        {"name", "jasper"},
        {"age", 20},
        {"tags", json::array({"cpp", "linux"})}
    });
    check(obj.is_object(), "object builder");
    check(obj["name"].as_string() == "jasper", "builder string val");
    check(obj["age"].as_int() == 20, "builder int val");
    check(obj["tags"][0].as_string() == "cpp", "builder nested array");
}

void test_compact_dump() {
    auto v = json::parse(R"({"x":1,"y":2})");
    std::string compact = json::dump(v, 0);
    check(compact.find('\n') == std::string::npos, "compact has no newlines");
    check(compact.find("\"x\"") != std::string::npos, "compact has key");
}

void test_unicode() {
    auto v = json::parse(R"({"msg":"élève"})");
    std::string s = v["msg"].as_string();
    check(s.size() == 7, "unicode byte length");
    check((unsigned char)s[0] == 0xC3 && (unsigned char)s[1] == 0xA9, "unicode utf-8 bytes");
}

void test_negative_number() {
    auto v = json::parse(R"({"temp":-42,"ratio":-3.14})");
    check(v["temp"].as_int() == -42, "negative int");
    check(std::abs(v["ratio"].as_number() - (-3.14)) < 0.001, "negative float");
}

void test_equality() {
    auto a = json::parse("42");
    auto b = json::parse("42");
    auto c = json::parse("43");
    check(a == b, "equal numbers");
    check(!(a == c), "not equal numbers");
}

int main() {
    test_basic();
    test_array();
    test_nested();
    test_null();
    test_empty();
    test_escapes();
    test_float();
    test_dump();
    test_bool_false();
    test_contains();
    test_get_typed();
    test_try_get();
    test_value_fallback();
    test_builders();
    test_compact_dump();
    test_unicode();
    test_negative_number();
    test_equality();

    std::cout << ok << "/" << (ok + fail) << " tests passed\n";
    return fail == 0 ? 0 : 1;
}
