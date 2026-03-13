#include <builtin/string.h>
#include <cctype>

namespace rumina {
namespace builtin {
namespace string {

Value concat(const std::vector<Value>& args) {
    std::string result;
    for (const auto& arg : args) {
        result += arg.toString();
    }
    return Value(result);
}

Value length(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("string::length expects 1 argument");
    }
    
    if (args[0].getType() != Value::Type::String) {
        throw std::runtime_error("string::length expects string, got " + args[0].typeName());
    }
    
    return Value(static_cast<int64_t>(args[0].getString().length()));
}

Value char_at(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("string::char_at expects 2 arguments (string, index)");
    }
    
    if (args[0].getType() != Value::Type::String || args[1].getType() != Value::Type::Int) {
        throw std::runtime_error("string::char_at expects (string, int)");
    }
    
    const std::string& s = args[0].getString();
    int64_t idx = args[1].getInt();
    
    if (idx < 0) idx = s.length() + idx;
    
    if (idx < 0 || static_cast<size_t>(idx) >= s.length()) {
        throw std::runtime_error("String index out of bounds: " + std::to_string(idx));
    }
    
    return Value(static_cast<int64_t>(static_cast<unsigned char>(s[idx])));
}

Value at(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("string::at expects 2 arguments (string, index)");
    }
    
    if (args[0].getType() != Value::Type::String || args[1].getType() != Value::Type::Int) {
        throw std::runtime_error("string::at expects (string, int)");
    }
    
    const std::string& s = args[0].getString();
    int64_t idx = args[1].getInt();
    
    if (idx < 0) idx = s.length() + idx;
    
    if (idx < 0 || static_cast<size_t>(idx) >= s.length()) {
        throw std::runtime_error("String index out of bounds: " + std::to_string(idx));
    }
    
    return Value(std::string(1, s[idx]));
}

Value find(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("string::find expects 3 arguments (string, start, substring)");
    }
    
    if (args[0].getType() != Value::Type::String || 
        args[1].getType() != Value::Type::Int ||
        args[2].getType() != Value::Type::String) {
        throw std::runtime_error("string::find expects (string, int, string)");
    }
    
    const std::string& s = args[0].getString();
    int64_t start = args[1].getInt();
    const std::string& sub = args[2].getString();
    
    if (start < 0 || static_cast<size_t>(start) > s.length()) {
        return Value(static_cast<int64_t>(-1)); // return Value(-1);
    }
    
    size_t pos = s.find(sub, static_cast<size_t>(start));
    if (pos == std::string::npos) {
        return Value(static_cast<int64_t>(-1)); // return Value(-1);
    }
    
    return Value(static_cast<int64_t>(pos));
}

Value sub(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("string::sub expects 3 arguments (string, start, length)");
    }
    
    if (args[0].getType() != Value::Type::String || 
        args[1].getType() != Value::Type::Int ||
        args[2].getType() != Value::Type::Int) {
        throw std::runtime_error("string::sub expects (string, int, int)");
    }
    
    const std::string& s = args[0].getString();
    int64_t start = args[1].getInt();
    int64_t len = args[2].getInt();
    
    if (start < 0) start = s.length() + start;
    if (start < 0) start = 0;
    if (static_cast<size_t>(start) >= s.length()) {
        return Value("");
    }
    
    size_t start_pos = static_cast<size_t>(start);
    size_t end_pos = start_pos + static_cast<size_t>(len);
    if (end_pos > s.length()) {
        end_pos = s.length();
    }
    
    return Value(s.substr(start_pos, end_pos - start_pos));
}

Value cat(const std::vector<Value>& args) {
    return concat(args);
}

Value replace_by_index(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("string::replace_by_index expects 3 arguments (string, start, replacement)");
    }
    
    if (args[0].getType() != Value::Type::String || 
        args[1].getType() != Value::Type::Int ||
        args[2].getType() != Value::Type::String) {
        throw std::runtime_error("string::replace_by_index expects (string, int, string)");
    }
    
    const std::string& s = args[0].getString();
    int64_t start = args[1].getInt();
    const std::string& replacement = args[2].getString();
    
    if (start < 0) start = s.length() + start;
    if (start < 0 || static_cast<size_t>(start) >= s.length()) {
        return Value(s);
    }
    
    size_t start_pos = static_cast<size_t>(start);
    size_t replacement_len = replacement.length();
    size_t end_pos = start_pos + replacement_len;
    if (end_pos > s.length()) {
        end_pos = s.length();
    }
    
    std::string result = s.substr(0, start_pos) + replacement + s.substr(end_pos);
    return Value(result);
}

} // namespace string
} // namespace builtin
} // namespace rumina
