#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <memory>
#include <cctype>
#include <sstream>
#include <stdexcept>

class JSONValue;

using JSONObject = std::unordered_map<std::string, JSONValue>;
using JSONArray  = std::vector<JSONValue>;

class JSONValue {
public:
    using ValueType = std::variant<
        std::nullptr_t,
        bool,
        double,
        std::string,
        JSONArray,
        JSONObject
    >;

    JSONValue() : value(nullptr) {}
    JSONValue(ValueType v) : value(std::move(v)) {}

    const ValueType& get() const { return value; }

private:
    ValueType value;
};

class JSONParser {
public:
    explicit JSONParser(const std::string& input)
        : input(input), pos(0) {}

    JSONValue parse() {
        skipWhitespace();
        JSONValue val = parseValue();
        skipWhitespace();
        if (pos != input.size())
            throw std::runtime_error("Unexpected trailing characters");
        return val;
    }

private:
    std::string input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.size() && std::isspace(input[pos]))
            pos++;
    }

    char peek() const {
        return pos < input.size() ? input[pos] : '\0';
    }

    char get() {
        return pos < input.size() ? input[pos++] : '\0';
    }

    JSONValue parseValue() {
        skipWhitespace();
        char c = peek();

        if (c == '"') return JSONValue(parseString());
        if (c == '{') return JSONValue(parseObject());
        if (c == '[') return JSONValue(parseArray());
        if (std::isdigit(c) || c == '-') return JSONValue(parseNumber());
        if (input.compare(pos, 4, "true") == 0) {
            pos += 4;
            return JSONValue(true);
        }
        if (input.compare(pos, 5, "false") == 0) {
            pos += 5;
            return JSONValue(false);
        }
        if (input.compare(pos, 4, "null") == 0) {
            pos += 4;
            return JSONValue(nullptr);
        }

        throw std::runtime_error("Invalid JSON value");
    }

    std::string parseString() {
        if (get() != '"')
            throw std::runtime_error("Expected '\"'");

        std::string result;
        while (pos < input.size()) {
            char c = get();
            if (c == '"') break;
            if (c == '\\') {
                char next = get();
                if (next == '"' || next == '\\' || next == '/')
                    result += next;
                else if (next == 'n')
                    result += '\n';
                else
                    throw std::runtime_error("Invalid escape sequence");
            } else {
                result += c;
            }
        }
        return result;
    }

    double parseNumber() {
        size_t start = pos;
        if (peek() == '-') pos++;

        while (std::isdigit(peek())) pos++;

        if (peek() == '.') {
            pos++;
            while (std::isdigit(peek())) pos++;
        }

        return std::stod(input.substr(start, pos - start));
    }

    JSONArray parseArray() {
        if (get() != '[')
            throw std::runtime_error("Expected '['");

        JSONArray arr;
        skipWhitespace();

        if (peek() == ']') {
            get();
            return arr;
        }

        while (true) {
            arr.push_back(parseValue());
            skipWhitespace();

            if (peek() == ']') {
                get();
                break;
            }

            if (get() != ',')
                throw std::runtime_error("Expected ',' in array");
        }

        return arr;
    }

    JSONObject parseObject() {
        if (get() != '{')
            throw std::runtime_error("Expected '{'");

        JSONObject obj;
        skipWhitespace();

        if (peek() == '}') {
            get();
            return obj;
        }

        while (true) {
            skipWhitespace();
            std::string key = parseString();
            skipWhitespace();

            if (get() != ':')
                throw std::runtime_error("Expected ':' after key");

            skipWhitespace();
            obj[key] = parseValue();
            skipWhitespace();

            if (peek() == '}') {
                get();
                break;
            }

            if (get() != ',')
                throw std::runtime_error("Expected ',' in object");
        }

        return obj;
    }
};

// ---------- Pretty Printer ----------

void printJSON(const JSONValue& val, int indent = 0) {
    const auto& v = val.get();

    std::visit([indent](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            std::cout << "null";
        }
        else if constexpr (std::is_same_v<T, bool>) {
            std::cout << (arg ? "true" : "false");
        }
        else if constexpr (std::is_same_v<T, double>) {
            std::cout << arg;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "\"" << arg << "\"";
        }
        else if constexpr (std::is_same_v<T, JSONArray>) {
            std::cout << "[\n";
            for (size_t i = 0; i < arg.size(); ++i) {
                std::cout << std::string(indent + 2, ' ');
                printJSON(arg[i], indent + 2);
                if (i != arg.size() - 1)
                    std::cout << ",";
                std::cout << "\n";
            }
            std::cout << std::string(indent, ' ') << "]";
        }
        else if constexpr (std::is_same_v<T, JSONObject>) {
            std::cout << "{\n";
            size_t count = 0;
            for (const auto& [key, value] : arg) {
                std::cout << std::string(indent + 2, ' ')
                          << "\"" << key << "\": ";
                printJSON(value, indent + 2);
                if (count++ != arg.size() - 1)
                    std::cout << ",";
                std::cout << "\n";
            }
            std::cout << std::string(indent, ' ') << "}";
        }
    }, v);
}

// ---------- Main ----------

int main() {
    std::string jsonText = R"(
    {
        "name": "SWSCODES",
        "age": 21,
        "isDeveloper": true,
        "skills": ["C++", "Systems", "Networking"],
        "projects": null
    }
    )";

    try {
        JSONParser parser(jsonText);
        JSONValue root = parser.parse();

        printJSON(root);
        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
    }

    return 0;
}
