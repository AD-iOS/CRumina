#include <builtin/buffer.h>

#include <cstring>
#include <algorithm>
#include <cctype>

namespace rumina {
namespace builtin {
namespace buffer {

constexpr const char* BUFFER_DATA_KEY = "__bytes";

// 创建Buffer模块
Value create_buffer_module() {
    auto ns = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*ns)["alloc"] = Value::makeNativeFunction("Buffer::alloc", buffer_alloc);
    (*ns)["from"] = Value::makeNativeFunction("Buffer::from", buffer_from);
    (*ns)["concat"] = Value::makeNativeFunction("Buffer::concat", buffer_concat);
    
    return Value::makeModule(ns);
}

// 从字节数组创建新的Buffer对象
Value new_buffer_from_bytes(const std::vector<uint8_t>& bytes) {
    auto fields = std::make_shared<std::unordered_map<std::string, Value>>();
    
    std::vector<Value> byte_values;
    byte_values.reserve(bytes.size());
    for (uint8_t b : bytes) {
        byte_values.push_back(Value(static_cast<int64_t>(b)));
    }
    
    (*fields)[BUFFER_DATA_KEY] = Value::makeArray(
        std::make_shared<std::vector<Value>>(std::move(byte_values)));
    
    (*fields)["length"] = Value::makeNativeFunction("Buffer::length", buffer_length);
    (*fields)["get"] = Value::makeNativeFunction("Buffer::get", buffer_get);
    (*fields)["set"] = Value::makeNativeFunction("Buffer::set", buffer_set);
    (*fields)["slice"] = Value::makeNativeFunction("Buffer::slice", buffer_slice);
    (*fields)["toText"] = Value::makeNativeFunction("Buffer::toText", buffer_to_text);
    (*fields)["toHex"] = Value::makeNativeFunction("Buffer::toHex", buffer_to_hex);
    (*fields)["toBase64"] = Value::makeNativeFunction("Buffer::toBase64", buffer_to_base64);
    (*fields)["toBase64Url"] = Value::makeNativeFunction("Buffer::toBase64Url", buffer_to_base64_url);
    (*fields)["copy"] = Value::makeNativeFunction("Buffer::copy", buffer_copy);
    (*fields)["fill"] = Value::makeNativeFunction("Buffer::fill", buffer_fill);
    (*fields)["indexOf"] = Value::makeNativeFunction("Buffer::indexOf", buffer_index_of);
    (*fields)["includes"] = Value::makeNativeFunction("Buffer::includes", buffer_includes);
    (*fields)["equals"] = Value::makeNativeFunction("Buffer::equals", buffer_equals);
    (*fields)["compare"] = Value::makeNativeFunction("Buffer::compare", buffer_compare);
    (*fields)["subarray"] = Value::makeNativeFunction("Buffer::subarray", buffer_subarray);
    
    return Value::makeStruct(fields);
}

// 将Buffer对象转换为字节数组
std::vector<uint8_t> buffer_to_bytes(const Value& value) {
    if (value.getType() != Value::Type::Struct) {
        throw std::runtime_error("Expected Buffer object");
    }
    
    auto fields = value.getStruct();
    auto it = fields->find(BUFFER_DATA_KEY);
    if (it == fields->end() || it->second.getType() != Value::Type::Array) {
        throw std::runtime_error("Expected Buffer object");
    }
    
    const auto& bytes_arr = *it->second.getArray();
    std::vector<uint8_t> result;
    result.reserve(bytes_arr.size());
    
    for (const auto& v : bytes_arr) {
        if (v.getType() != Value::Type::Int) {
            throw std::runtime_error("Byte must be int");
        }
        int64_t n = v.getInt();
        if (n < 0 || n > 255) {
            throw std::runtime_error("Byte value out of range: " + std::to_string(n));
        }
        result.push_back(static_cast<uint8_t>(n));
    }
    
    return result;
}

// 获取Buffer的内部数组
static std::shared_ptr<std::vector<Value>> get_buffer_array(const Value& value) {
    if (value.getType() != Value::Type::Struct) {
        throw std::runtime_error("Expected Buffer object");
    }
    
    auto fields = value.getStruct();
    auto it = fields->find(BUFFER_DATA_KEY);
    if (it == fields->end() || it->second.getType() != Value::Type::Array) {
        throw std::runtime_error("Expected Buffer object");
    }
    
    return it->second.getArray();
}

// 将Value转换为uint8_t
static uint8_t value_to_u8(const Value& v) {
    if (v.getType() != Value::Type::Int) {
        throw std::runtime_error("Byte must be int");
    }
    int64_t n = v.getInt();
    if (n < 0 || n > 255) {
        throw std::runtime_error("Byte value out of range: " + std::to_string(n));
    }
    return static_cast<uint8_t>(n);
}

// 获取索引参数
static size_t get_index(const std::vector<Value>& args, size_t pos) {
    if (pos >= args.size()) {
        throw std::runtime_error("Missing index argument");
    }
    int64_t idx = args[pos].toInt();
    if (idx < 0) {
        throw std::runtime_error("Index must be non-negative");
    }
    return static_cast<size_t>(idx);
}

// 解码十六进制字符串（Node.js风格）
std::vector<uint8_t> decode_hex_nodeish(const std::string& s) {
    std::vector<uint8_t> out;
    std::optional<uint8_t> hi;
    
    for (char c : s) {
        uint8_t val;
        if (c >= '0' && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            val = 10 + (c - 'A');
        } else {
            break;
        }
        
        if (hi.has_value()) {
            out.push_back((hi.value() << 4) | val);
            hi.reset();
        } else {
            hi = val;
        }
    }
    
    return out;
}

// 解码Base64字符串（Node.js风格）
std::vector<uint8_t> decode_base64_nodeish(const std::string& s) {
    std::string t;
    for (char c : s) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            t += c;
        }
    }
    
    for (char& c : t) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    
    size_t padding = (4 - (t.size() % 4)) % 4;
    for (size_t i = 0; i < padding; ++i) {
        t += '=';
    }
    
    static const std::string b64_table = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::vector<uint8_t> out;
    for (size_t i = 0; i < t.size(); i += 4) {
        if (i + 3 >= t.size()) break;
        
        uint32_t val = 0;
        for (size_t j = 0; j < 4; ++j) {
            char c = t[i + j];
            if (c == '=') {
                val <<= 6;
                continue;
            }
            size_t pos = b64_table.find(c);
            if (pos == std::string::npos) {
                throw std::runtime_error("Invalid base64 character");
            }
            val = (val << 6) | static_cast<uint32_t>(pos);
        }
        
        out.push_back((val >> 16) & 0xFF);
        if (t[i + 2] != '=') {
            out.push_back((val >> 8) & 0xFF);
        }
        if (t[i + 3] != '=') {
            out.push_back(val & 0xFF);
        }
    }
    
    return out;
}

// 从Value中搜索模式
static std::vector<uint8_t> search_pattern_from_value(const Value& value, 
                                                       const std::optional<std::string>& encoding) {
    if (value.getType() == Value::Type::Int) {
        uint8_t b = value_to_u8(value);
        return {b};
    } else if (value.getType() == Value::Type::String) {
        const std::string& s = value.getString();
        std::string enc = encoding.value_or("utf8");
        
        if (enc == "utf8" || enc == "utf-8") {
            return std::vector<uint8_t>(s.begin(), s.end());
        } else if (enc == "hex") {
            return decode_hex_nodeish(s);
        } else if (enc == "base64" || enc == "base64url") {
            return decode_base64_nodeish(s);
        } else {
            throw std::runtime_error("Unsupported encoding: " + enc);
        }
    } else if (value.getType() == Value::Type::Struct) {
        return buffer_to_bytes(value);
    } else {
        throw std::runtime_error("pattern must be Int/String/Buffer");
    }
}

// 查找子数组
static std::optional<size_t> find_subslice(const std::vector<uint8_t>& haystack,
                                           const std::vector<uint8_t>& needle,
                                           size_t offset) {
    if (needle.empty()) {
        return offset <= haystack.size() ? offset : haystack.size();
    }
    if (offset >= haystack.size() || needle.size() > haystack.size() - offset) {
        return std::nullopt;
    }
    
    for (size_t i = offset; i <= haystack.size() - needle.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < needle.size(); ++j) {
            if (haystack[i + j] != needle[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return i;
        }
    }
    return std::nullopt;
}

// 解析字节偏移量
static size_t parse_byte_offset(size_t len, const std::optional<Value>& offset_value) {
    if (!offset_value.has_value()) {
        return 0;
    }
    
    int64_t raw = offset_value->toInt();
    if (raw >= 0) {
        return static_cast<size_t>(raw);
    } else {
        size_t from_end = len - std::min(static_cast<size_t>(-raw), len);
        return from_end;
    }
}

// 解析编码参数
static std::optional<std::string> parse_encoding_arg(const std::optional<Value>& value,
                                                      const std::string& fn_name) {
    if (!value.has_value()) {
        return std::nullopt;
    }
    
    if (value->getType() == Value::Type::String) {
        return value->getString();
    } else {
        throw std::runtime_error(fn_name + " encoding must be string");
    }
}

// Buffer.alloc(size)
Value buffer_alloc(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Buffer.alloc expects 1 argument (size)");
    }
    
    int64_t size = args[0].toInt();
    if (size < 0) {
        throw std::runtime_error("Buffer size must be non-negative");
    }
    
    return new_buffer_from_bytes(std::vector<uint8_t>(static_cast<size_t>(size), 0));
}

// Buffer.from(data, [encoding])
Value buffer_from(const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        throw std::runtime_error("Buffer.from expects 1 or 2 arguments (data, [encoding])");
    }
    
    std::string encoding = "utf8";
    if (args.size() == 2) {
        if (args[1].getType() != Value::Type::String) {
            throw std::runtime_error("Buffer.from encoding must be string");
        }
        encoding = args[1].getString();
    }
    
    const Value& data = args[0];
    
    if (data.getType() == Value::Type::String) {
        const std::string& s = data.getString();
        std::vector<uint8_t> bytes;
        
        if (encoding == "utf8" || encoding == "utf-8") {
            bytes.assign(s.begin(), s.end());
        } else if (encoding == "hex") {
            bytes = decode_hex_nodeish(s);
        } else if (encoding == "base64" || encoding == "base64url") {
            bytes = decode_base64_nodeish(s);
        } else {
            throw std::runtime_error("Unsupported encoding: " + encoding);
        }
        
        return new_buffer_from_bytes(bytes);
    } else if (data.getType() == Value::Type::Array) {
        const auto& arr = *data.getArray();
        std::vector<uint8_t> bytes;
        bytes.reserve(arr.size());
        
        for (const auto& v : arr) {
            bytes.push_back(value_to_u8(v));
        }
        
        return new_buffer_from_bytes(bytes);
    } else if (data.getType() == Value::Type::Struct) {
        std::vector<uint8_t> bytes = buffer_to_bytes(data);
        return new_buffer_from_bytes(bytes);
    } else {
        throw std::runtime_error("Buffer.from data must be String, List<Int>, or Buffer");
    }
}

// Buffer.concat(buffers)
Value buffer_concat(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Buffer.concat expects 1 argument (buffers)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("Buffer.concat expects List<Buffer>");
    }
    
    const auto& list = *args[0].getArray();
    std::vector<uint8_t> out;
    
    for (const auto& item : list) {
        std::vector<uint8_t> bytes = buffer_to_bytes(item);
        out.insert(out.end(), bytes.begin(), bytes.end());
    }
    
    return new_buffer_from_bytes(out);
}

// buffer.length
Value buffer_length(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Buffer.length expects no arguments");
    }
    
    auto arr = get_buffer_array(args[0]);
    return Value(static_cast<int64_t>(arr->size()));
}

// buffer.get(index)
Value buffer_get(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("Buffer.get expects 1 argument (index)");
    }
    
    auto arr = get_buffer_array(args[0]);
    size_t idx = get_index(args, 1);
    
    if (idx >= arr->size()) {
        throw std::runtime_error("Buffer index out of bounds: " + std::to_string(idx));
    }
    
    return (*arr)[idx];
}

// buffer.set(index, value)
Value buffer_set(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("Buffer.set expects 2 arguments (index, value)");
    }
    
    auto arr = get_buffer_array(args[0]);
    size_t idx = get_index(args, 1);
    uint8_t value = value_to_u8(args[2]);
    
    if (idx >= arr->size()) {
        throw std::runtime_error("Buffer index out of bounds: " + std::to_string(idx));
    }
    
    (*arr)[idx] = Value(static_cast<int64_t>(value));
    return Value();
}

// buffer.slice(start, end)
Value buffer_slice(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("Buffer.slice expects 2 arguments (start, end)");
    }
    
    auto arr = get_buffer_array(args[0]);
    size_t start = get_index(args, 1);
    size_t end = get_index(args, 2);
    
    if (start > end || end > arr->size()) {
        throw std::runtime_error("Invalid slice range: start=" + std::to_string(start) +
                                 ", end=" + std::to_string(end) +
                                 ", length=" + std::to_string(arr->size()));
    }
    
    std::vector<uint8_t> out;
    out.reserve(end - start);
    for (size_t i = start; i < end; ++i) {
        out.push_back(value_to_u8((*arr)[i]));
    }
    
    return new_buffer_from_bytes(out);
}

// buffer.toText()
Value buffer_to_text(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Buffer.toText expects no arguments");
    }
    
    std::vector<uint8_t> bytes = buffer_to_bytes(args[0]);
    std::string text(bytes.begin(), bytes.end());
    return Value(text);
}

// buffer.toHex()
Value buffer_to_hex(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Buffer.toHex expects no arguments");
    }
    
    std::vector<uint8_t> bytes = buffer_to_bytes(args[0]);
    std::string hex;
    hex.reserve(bytes.size() * 2);
    
    static const char* hex_chars = "0123456789abcdef";
    for (uint8_t b : bytes) {
        hex += hex_chars[b >> 4];
        hex += hex_chars[b & 0x0F];
    }
    
    return Value(hex);
}

// buffer.toBase64()
Value buffer_to_base64(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Buffer.toBase64 expects no arguments");
    }
    
    std::vector<uint8_t> bytes = buffer_to_bytes(args[0]);
    
    static const char* b64_table = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    size_t i = 0;
    while (i < bytes.size()) {
        uint32_t octet_a = i < bytes.size() ? bytes[i++] : 0;
        uint32_t octet_b = i < bytes.size() ? bytes[i++] : 0;
        uint32_t octet_c = i < bytes.size() ? bytes[i++] : 0;
        
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        result += b64_table[(triple >> 18) & 0x3F];
        result += b64_table[(triple >> 12) & 0x3F];
        result += b64_table[(triple >> 6) & 0x3F];
        result += b64_table[triple & 0x3F];
    }
    
    size_t padding = 3 - (bytes.size() % 3);
    if (padding < 3) {
        for (size_t j = 0; j < padding; ++j) {
            result[result.length() - 1 - j] = '=';
        }
    }
    
    return Value(result);
}

// buffer.toBase64Url()
Value buffer_to_base64_url(const std::vector<Value>& args) {
    Value base64 = buffer_to_base64(args);
    std::string result = base64.getString();
    
    for (char& c : result) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    
    while (!result.empty() && result.back() == '=') {
        result.pop_back();
    }
    
    return Value(result);
}

// buffer.copy(target, [targetStart], [sourceStart], [sourceEnd])
Value buffer_copy(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 5) {
        throw std::runtime_error("Buffer.copy expects 1-4 arguments (target, [targetStart], [sourceStart], [sourceEnd])");
    }
    
    auto source_arr = get_buffer_array(args[0]);
    auto target_arr = get_buffer_array(args[1]);
    
    size_t target_start = (args.size() >= 3) ? get_index(args, 2) : 0;
    size_t source_start = (args.size() >= 4) ? get_index(args, 3) : 0;
    size_t source_end = (args.size() >= 5) ? get_index(args, 4) : source_arr->size();
    
    if (source_start > source_end || source_end > source_arr->size()) {
        throw std::runtime_error("Invalid source range: start=" + std::to_string(source_start) +
                                 ", end=" + std::to_string(source_end) +
                                 ", length=" + std::to_string(source_arr->size()));
    }
    
    if (target_start > target_arr->size()) {
        throw std::runtime_error("Target start out of bounds: start=" + std::to_string(target_start) +
                                 ", length=" + std::to_string(target_arr->size()));
    }
    
    size_t copied = 0;
    for (size_t i = source_start; i < source_end && target_start + copied < target_arr->size(); ++i) {
        (*target_arr)[target_start + copied] = (*source_arr)[i];
        ++copied;
    }
    
    return Value(static_cast<int64_t>(copied));
}

// buffer.fill(value, [start], [end])
Value buffer_fill(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 4) {
        throw std::runtime_error("Buffer.fill expects 1-3 arguments (value, [start], [end])");
    }
    
    auto arr = get_buffer_array(args[0]);
    uint8_t value = value_to_u8(args[1]);
    
    size_t len = arr->size();
    size_t start = (args.size() >= 3) ? get_index(args, 2) : 0;
    size_t end = (args.size() >= 4) ? get_index(args, 3) : len;
    
    if (start > end || end > len) {
        throw std::runtime_error("Invalid fill range: start=" + std::to_string(start) +
                                 ", end=" + std::to_string(end) +
                                 ", length=" + std::to_string(len));
    }
    
    for (size_t i = start; i < end; ++i) {
        (*arr)[i] = Value(static_cast<int64_t>(value));
    }
    
    return Value();
}

// buffer.indexOf(pattern, [offset], [encoding])
Value buffer_index_of(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 4) {
        throw std::runtime_error("Buffer.indexOf expects 1-3 arguments (pattern, [offset], [encoding])");
    }
    
    std::vector<uint8_t> bytes = buffer_to_bytes(args[0]);
    size_t offset = parse_byte_offset(bytes.size(), 
        args.size() >= 3 ? std::optional<Value>(args[2]) : std::nullopt);
    std::optional<std::string> enc = parse_encoding_arg(
        args.size() >= 4 ? std::optional<Value>(args[3]) : std::nullopt,
        "Buffer.indexOf");
    
    std::vector<uint8_t> pat = search_pattern_from_value(args[1], enc);
    
    auto pos = find_subslice(bytes, pat, offset);
    if (pos.has_value()) {
        return Value(static_cast<int64_t>(pos.value()));
    } else {
        return Value(static_cast<int64_t>(-1)); // return Value(-1);
    }
}

// buffer.includes(pattern, [offset], [encoding])
Value buffer_includes(const std::vector<Value>& args) {
    Value idx = buffer_index_of(args);
    return Value(idx.getInt() >= 0);
}

// buffer.equals(other)
Value buffer_equals(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("Buffer.equals expects 1 argument (other)");
    }
    
    std::vector<uint8_t> a = buffer_to_bytes(args[0]);
    std::vector<uint8_t> b = buffer_to_bytes(args[1]);
    
    return Value(a == b);
}

// buffer.compare(other, [targetStart], [targetEnd], [sourceStart], [sourceEnd])
Value buffer_compare(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 6) {
        throw std::runtime_error("Buffer.compare expects 1-5 arguments (other, [targetStart], [targetEnd], [sourceStart], [sourceEnd])");
    }
    
    std::vector<uint8_t> a = buffer_to_bytes(args[0]);
    std::vector<uint8_t> b = buffer_to_bytes(args[1]);
    
    size_t target_start = (args.size() >= 3) ? get_index(args, 2) : 0;
    size_t target_end = (args.size() >= 4) ? get_index(args, 3) : b.size();
    size_t source_start = (args.size() >= 5) ? get_index(args, 4) : 0;
    size_t source_end = (args.size() >= 6) ? get_index(args, 5) : a.size();
    
    if (target_start > target_end || target_end > b.size()) {
        throw std::runtime_error("Invalid target compare range: start=" + std::to_string(target_start) +
                                 ", end=" + std::to_string(target_end) +
                                 ", length=" + std::to_string(b.size()));
    }
    if (source_start > source_end || source_end > a.size()) {
        throw std::runtime_error("Invalid source compare range: start=" + std::to_string(source_start) +
                                 ", end=" + std::to_string(source_end) +
                                 ", length=" + std::to_string(a.size()));
    }
    
    const auto* a_ptr = a.data() + source_start;
    const auto* b_ptr = b.data() + target_start;
    size_t a_len = source_end - source_start;
    size_t b_len = target_end - target_start;
    
    int cmp = std::memcmp(a_ptr, b_ptr, std::min(a_len, b_len));
    if (cmp == 0) {
        if (a_len < b_len) cmp = -1;
        else if (a_len > b_len) cmp = 1;
    }
    
    return Value(static_cast<int64_t>(cmp));
}

// 规范化范围索引
static size_t normalize_range_index(size_t len, int64_t idx) {
    if (idx >= 0) {
        return std::min(static_cast<size_t>(idx), len);
    } else {
        return len - std::min(static_cast<size_t>(-idx), len);
    }
}

// buffer.subarray(start, [end])
Value buffer_subarray(const std::vector<Value>& args) {
    if (args.size() != 2 && args.size() != 3) {
        throw std::runtime_error("Buffer.subarray expects 1 or 2 arguments (start, [end])");
    }
    
    std::vector<uint8_t> bytes = buffer_to_bytes(args[0]);
    size_t len = bytes.size();
    
    size_t start = normalize_range_index(len, args[1].toInt());
    size_t end = (args.size() == 3) ? normalize_range_index(len, args[2].toInt()) : len;
    
    if (end < start) {
        return new_buffer_from_bytes({});
    }
    
    std::vector<uint8_t> result(bytes.begin() + start, bytes.begin() + end);
    return new_buffer_from_bytes(result);
}

} // namespace buffer
} // namespace builtin
} // namespace rumina
