#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <charconv>
#include <cmath>
#include <sstream>
#include <ostream>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <type_traits>

namespace json {

struct Value;

using Null    = std::monostate;
using Bool    = bool;
using Number  = double;
using String  = std::string;
using Array   = std::vector<Value>;
using Object  = std::unordered_map<std::string, Value>;

struct Value {
    std::variant<Null, Bool, Number, String, Array, Object> data;

    Value() : data(Null{}) {}
    Value(bool b) : data(b) {}
    Value(double n) : data(n) {}
    Value(int n) : data((double)n) {}
    Value(long long n) : data((double)n) {}
    Value(std::nullptr_t) : data(Null{}) {}
    Value(const char* s) : data(String(s)) {}
    Value(String s) : data(std::move(s)) {}
    Value(Array a) : data(std::move(a)) {}
    Value(Object o) : data(std::move(o)) {}

    bool is_null()   const { return std::holds_alternative<Null>(data); }
    bool is_bool()   const { return std::holds_alternative<Bool>(data); }
    bool is_number() const { return std::holds_alternative<Number>(data); }
    bool is_string() const { return std::holds_alternative<String>(data); }
    bool is_array()  const { return std::holds_alternative<Array>(data); }
    bool is_object() const { return std::holds_alternative<Object>(data); }

    bool        as_bool()   const { return std::get<Bool>(data); }
    double      as_number() const { return std::get<Number>(data); }
    int         as_int()    const { return (int)std::get<Number>(data); }
    const String& as_string() const { return std::get<String>(data); }
    const Array&  as_array()  const { return std::get<Array>(data); }
    const Object& as_object() const { return std::get<Object>(data); }

    Array&  as_array()  { return std::get<Array>(data); }
    Object& as_object() { return std::get<Object>(data); }

    Value& operator[](const std::string& key) {
        return std::get<Object>(data)[key];
    }
    const Value& operator[](const std::string& key) const {
        return std::get<Object>(data).at(key);
    }

    auto begin()       { return std::get<Array>(data).begin(); }
    auto end()         { return std::get<Array>(data).end(); }
    auto begin() const { return std::get<Array>(data).cbegin(); }
    auto end()   const { return std::get<Array>(data).cend(); }
    Value& operator[](size_t i) {
        return std::get<Array>(data)[i];
    }
    const Value& operator[](size_t i) const {
        return std::get<Array>(data)[i];
    }

    bool contains(const std::string& key) const {
        if (!is_object()) return false;
        return std::get<Object>(data).count(key) > 0;
    }

    size_t size() const {
        if (is_array())  return std::get<Array>(data).size();
        if (is_object()) return std::get<Object>(data).size();
        return 0;
    }

    bool empty() const {
        if (is_array())  return std::get<Array>(data).empty();
        if (is_object()) return std::get<Object>(data).empty();
        if (is_string()) return std::get<String>(data).empty();
        return true;
    }

    const Value* try_get(const std::string& key) const {
        if (!is_object()) return nullptr;
        const auto& obj = std::get<Object>(data);
        auto it = obj.find(key);
        return it != obj.end() ? &it->second : nullptr;
    }

    Value value(const std::string& key, Value default_val) const {
        const Value* p = try_get(key);
        return p ? *p : std::move(default_val);
    }

    template<typename T>
    T get(const std::string& key, T def = T{}) const {
        const Value* p = try_get(key);
        if (!p) return def;
        if constexpr (std::is_same_v<T, bool>) {
            return p->is_bool() ? p->as_bool() : def;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return p->is_string() ? p->as_string() : def;
        } else if constexpr (std::is_floating_point_v<T>) {
            return p->is_number() ? static_cast<T>(p->as_number()) : def;
        } else if constexpr (std::is_integral_v<T>) {
            return p->is_number() ? static_cast<T>(p->as_number()) : def;
        } else {
            return def;
        }
    }

    bool operator==(const Value& other) const {
        if (data.index() != other.data.index()) return false;
        if (is_null())   return true;
        if (is_bool())   return as_bool()   == other.as_bool();
        if (is_number()) return as_number() == other.as_number();
        if (is_string()) return as_string() == other.as_string();
        if (is_array()) {
            const auto& a = as_array();
            const auto& b = other.as_array();
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); i++)
                if (!(a[i] == b[i])) return false;
            return true;
        }
        if (is_object()) {
            const auto& a = as_object();
            const auto& b = other.as_object();
            if (a.size() != b.size()) return false;
            for (const auto& [k, v] : a) {
                auto it = b.find(k);
                if (it == b.end() || !(v == it->second)) return false;
            }
            return true;
        }
        return false;
    }
    bool operator!=(const Value& other) const { return !(*this == other); }

    bool operator==(const std::string& s)  const { return is_string() && as_string() == s; }
    bool operator==(const char* s)         const { return is_string() && as_string() == s; }
    bool operator==(double n)              const { return is_number() && as_number() == n; }
    bool operator==(int n)                 const { return is_number() && as_number() == (double)n; }
    bool operator==(bool b)                const { return is_bool()   && as_bool()   == b; }
    bool operator==(std::nullptr_t)        const { return is_null(); }

    bool operator!=(const std::string& s)  const { return !(*this == s); }
    bool operator!=(const char* s)         const { return !(*this == s); }
    bool operator!=(double n)              const { return !(*this == n); }
    bool operator!=(int n)                 const { return !(*this == n); }
    bool operator!=(bool b)                const { return !(*this == b); }
    bool operator!=(std::nullptr_t)        const { return !(*this == nullptr); }
};

class ParseError : public std::runtime_error {
public:
    int line, col;
    ParseError(const std::string& msg, int l, int c)
        : std::runtime_error(msg + " (line " + std::to_string(l) + ", col " + std::to_string(c) + ")")
        , line(l), col(c) {}
};

namespace detail {

struct Parser {
    const char* p;
    const char* end;
    int line = 1, col = 1;

    explicit Parser(const char* str, size_t len) : p(str), end(str + len) {}

    char peek() const { return p < end ? *p : '\0'; }

    char consume() {
        char c = *p++;
        if (c == '\n') { line++; col = 1; }
        else col++;
        return c;
    }

    void skip_ws() {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
            consume();
    }

    [[noreturn]] void error(const std::string& msg) {
        throw ParseError(msg, line, col);
    }

    void expect(char c) {
        skip_ws();
        if (peek() != c) error(std::string("expected '") + c + "' got '" + peek() + "'");
        consume();
    }

    void match_literal(const char* expected) {
        while (*expected) {
            if (p >= end) error("unexpected end of input");
            if (peek() != *expected) error(std::string("invalid token, expected '") + *expected + "'");
            consume();
            ++expected;
        }
    }

    uint32_t parse_hex4() {
        uint32_t v = 0;
        for (int i = 0; i < 4; i++) {
            if (p >= end) error("unexpected end in unicode escape");
            char c = consume();
            v <<= 4;
            if      (c >= '0' && c <= '9') v |= (uint32_t)(c - '0');
            else if (c >= 'a' && c <= 'f') v |= (uint32_t)(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') v |= (uint32_t)(c - 'A' + 10);
            else error(std::string("invalid hex digit '") + c + "' in unicode escape");
        }
        return v;
    }

    static void append_utf8(std::string& s, uint32_t cp) {
        if (cp <= 0x7F) {
            s += (char)cp;
        } else if (cp <= 0x7FF) {
            s += (char)(0xC0 | (cp >> 6));
            s += (char)(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
            s += (char)(0xE0 | (cp >> 12));
            s += (char)(0x80 | ((cp >> 6) & 0x3F));
            s += (char)(0x80 | (cp & 0x3F));
        } else {
            s += (char)(0xF0 | (cp >> 18));
            s += (char)(0x80 | ((cp >> 12) & 0x3F));
            s += (char)(0x80 | ((cp >> 6) & 0x3F));
            s += (char)(0x80 | (cp & 0x3F));
        }
    }

    Value parse_value() {
        skip_ws();
        char c = peek();
        if (c == '{') return parse_object();
        if (c == '[') return parse_array();
        if (c == '"') return parse_string();
        if (c == 't') { match_literal("true");  return Value(true);  }
        if (c == 'f') { match_literal("false"); return Value(false); }
        if (c == 'n') { match_literal("null");  return Value();      }
        if (c == '-' || (c >= '0' && c <= '9')) return parse_number();
        error(std::string("unexpected character '") + c + "'");
    }

    Value parse_object() {
        expect('{');
        Object obj;
        skip_ws();
        if (peek() == '}') { consume(); return Value(std::move(obj)); }
        while (true) {
            skip_ws();
            if (peek() != '"') error("expected string key");
            String key = std::get<String>(parse_string().data);
            expect(':');
            obj[key] = parse_value();
            skip_ws();
            if (peek() == '}') { consume(); break; }
            if (peek() != ',') error("expected ',' or '}'");
            consume();
        }
        return Value(std::move(obj));
    }

    Value parse_array() {
        expect('[');
        Array arr;
        skip_ws();
        if (peek() == ']') { consume(); return Value(std::move(arr)); }
        while (true) {
            arr.push_back(parse_value());
            skip_ws();
            if (peek() == ']') { consume(); break; }
            if (peek() != ',') error("expected ',' or ']'");
            consume();
        }
        return Value(std::move(arr));
    }

    Value parse_string() {
        expect('"');
        std::string s;
        while (p < end) {
            char c = consume();
            if (c == '"') return Value(std::move(s));
            if (c == '\\') {
                char e = consume();
                switch (e) {
                    case '"': s += '"'; break;
                    case '\\': s += '\\'; break;
                    case '/': s += '/'; break;
                    case 'n': s += '\n'; break;
                    case 'r': s += '\r'; break;
                    case 't': s += '\t'; break;
                    case 'b': s += '\b'; break;
                    case 'f': s += '\f'; break;
                    case 'u': {
                        uint32_t cp = parse_hex4();
                        if (cp >= 0xD800 && cp <= 0xDBFF) {
                            if ((end - p) >= 2 && p[0] == '\\' && p[1] == 'u') {
                                consume();
                                consume();
                                uint32_t lo = parse_hex4();
                                if (lo >= 0xDC00 && lo <= 0xDFFF) {
                                    cp = 0x10000u + ((cp - 0xD800u) << 10) + (lo - 0xDC00u);
                                } else {
                                    append_utf8(s, 0xFFFD);
                                    cp = (lo >= 0xD800 && lo <= 0xDFFF) ? 0xFFFD : lo;
                                }
                            } else {
                                cp = 0xFFFD;
                            }
                        } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                            cp = 0xFFFD;
                        }
                        append_utf8(s, cp);
                        break;
                    }
                    default: s += e;
                }
            } else {
                s += c;
            }
        }
        error("unterminated string");
    }

    Value parse_number() {
        const char* start = p;
        if (peek() == '-') consume();
        if (p >= end || !(peek() >= '0' && peek() <= '9'))
            error("invalid number: expected digit");
        if (peek() == '0') {
            consume();
            if (peek() >= '0' && peek() <= '9')
                error("invalid number: leading zeros not allowed");
        } else {
            while (p < end && peek() >= '0' && peek() <= '9') consume();
        }
        if (peek() == '.') {
            consume();
            if (p >= end || !(peek() >= '0' && peek() <= '9'))
                error("invalid number: expected digit after '.'");
            while (p < end && peek() >= '0' && peek() <= '9') consume();
        }
        if (peek() == 'e' || peek() == 'E') {
            consume();
            if (peek() == '+' || peek() == '-') consume();
            if (p >= end || !(peek() >= '0' && peek() <= '9'))
                error("invalid number: expected digit in exponent");
            while (p < end && peek() >= '0' && peek() <= '9') consume();
        }
        double val;
        auto [ptr, ec] = std::from_chars(start, p, val);
        if (ec != std::errc{}) {
            try {
                val = std::stod(std::string(start, p));
            } catch (...) {
                error("invalid number");
            }
        }
        return Value(val);
    }
};

inline std::string dump(const Value& v, int indent, int depth);

inline std::string dump_string(const std::string& s) {
    std::string r = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"':  r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n";  break;
            case '\r': r += "\\r";  break;
            case '\t': r += "\\t";  break;
            case '\b': r += "\\b";  break;
            case '\f': r += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)c);
                    r += buf;
                } else {
                    r += (char)c;
                }
        }
    }
    return r + "\"";
}

inline std::string dump(const Value& v, int indent, int depth) {
    if (v.is_null())   return "null";
    if (v.is_bool())   return v.as_bool() ? "true" : "false";
    if (v.is_string()) return dump_string(v.as_string());
    if (v.is_number()) {
        double n = v.as_number();
        constexpr double safe_int_limit = 9007199254740992.0;
        if (std::isfinite(n) && n >= -safe_int_limit && n <= safe_int_limit && n == (long long)n)
            return std::to_string((long long)n);
        char buf[32];
        auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), n);
        if (ec == std::errc{}) return std::string(buf, ptr);
        std::ostringstream ss;
        ss.precision(std::numeric_limits<double>::max_digits10);
        ss << n;
        return ss.str();
    }
    if (v.is_array()) {
        const auto& arr = v.as_array();
        if (arr.empty()) return "[]";
        if (indent == 0) {
            std::string s = "[";
            for (size_t i = 0; i < arr.size(); i++) {
                s += dump(arr[i], 0, 0);
                if (i + 1 < arr.size()) s += ",";
            }
            return s + "]";
        }
        std::string pad(indent * depth, ' ');
        std::string pad1(indent * (depth + 1), ' ');
        std::string s = "[\n";
        for (size_t i = 0; i < arr.size(); i++) {
            s += pad1 + dump(arr[i], indent, depth + 1);
            if (i + 1 < arr.size()) s += ",";
            s += "\n";
        }
        return s + pad + "]";
    }
    if (v.is_object()) {
        const auto& obj = v.as_object();
        if (obj.empty()) return "{}";
        std::vector<std::string> keys;
        keys.reserve(obj.size());
        for (const auto& [k, _v] : obj) keys.push_back(k);
        std::sort(keys.begin(), keys.end());
        if (indent == 0) {
            std::string s = "{";
            for (size_t i = 0; i < keys.size(); i++) {
                s += dump_string(keys[i]) + ":" + dump(obj.at(keys[i]), 0, 0);
                if (i + 1 < keys.size()) s += ",";
            }
            return s + "}";
        }
        std::string pad(indent * depth, ' ');
        std::string pad1(indent * (depth + 1), ' ');
        std::string s = "{\n";
        for (size_t i = 0; i < keys.size(); i++) {
            s += pad1 + dump_string(keys[i]) + ": " + dump(obj.at(keys[i]), indent, depth + 1);
            if (i + 1 < keys.size()) s += ",";
            s += "\n";
        }
        return s + pad + "}";
    }
    return "null";
}

} // namespace detail

inline Value parse(std::string_view s) {
    detail::Parser p(s.data(), s.size());
    Value v = p.parse_value();
    p.skip_ws();
    if (p.p < p.end) p.error(std::string("unexpected trailing character '") + *p.p + "'");
    return v;
}

inline Value parse(const std::string& s) {
    return parse(std::string_view(s));
}

inline Value parse(const char* s) {
    return parse(std::string_view(s));
}

inline std::string dump(const Value& v, int indent = 2) {
    return detail::dump(v, indent, 0);
}

inline std::ostream& operator<<(std::ostream& os, const Value& v) {
    return os << dump(v);
}

inline Value object(std::initializer_list<std::pair<const std::string, Value>> init) {
    return Value(Object(init.begin(), init.end()));
}

inline Value array(std::initializer_list<Value> init) {
    return Value(Array(init.begin(), init.end()));
}

} // namespace json


