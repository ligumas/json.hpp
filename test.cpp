#include "json.hpp"
#include <cassert>
#include <iostream>

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

    std::cout << ok << "/" << (ok + fail) << " tests passed\n";
    return fail == 0 ? 0 : 1;
}
