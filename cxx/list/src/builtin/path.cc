#include <builtin/path.h>

#include <filesystem>

namespace rumina {
namespace builtin {
namespace path {

namespace fs = std::filesystem;

// 创建路径模块
Value create_path_module() {
    auto ns = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*ns)["join"] = Value::makeNativeFunction("path::join", path_join);
    (*ns)["basename"] = Value::makeNativeFunction("path::basename", path_basename);
    (*ns)["dirname"] = Value::makeNativeFunction("path::dirname", path_dirname);
    (*ns)["extname"] = Value::makeNativeFunction("path::extname", path_extname);
    (*ns)["isAbsolute"] = Value::makeNativeFunction("path::isAbsolute", path_is_absolute);
    (*ns)["normalize"] = Value::makeNativeFunction("path::normalize", path_normalize);
    (*ns)["resolve"] = Value::makeNativeFunction("path::resolve", path_resolve);
    (*ns)["relative"] = Value::makeNativeFunction("path::relative", path_relative);
    (*ns)["parse"] = Value::makeNativeFunction("path::parse", path_parse);
    (*ns)["format"] = Value::makeNativeFunction("path::format", path_format);
    
    (*ns)["sep"] = Value(std::string(1, fs::path::preferred_separator));
    (*ns)["delimiter"] = Value(
#ifdef _WIN32
        ";"
#else
        ":"
#endif
    );
    
    return Value::makeModule(ns);
}

// 将参数转换为字符串
static std::string as_string(const Value& v, const std::string& name) {
    if (v.getType() != Value::Type::String) {
        throw std::runtime_error(name + " expects string argument");
    }
    return v.getString();
}

// path.join(paths)
Value path_join(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.join expects 1 argument (paths)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("path.join expects List<String>");
    }
    
    const auto& parts = *args[0].getArray();
    fs::path result;
    
    for (const auto& part : parts) {
        if (part.getType() != Value::Type::String) {
            throw std::runtime_error("path.join expects List<String>");
        }
        result /= part.getString();
    }
    
    return Value(result.string());
}

// path.basename(path)
Value path_basename(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.basename expects 1 argument (path)");
    }
    
    std::string p = as_string(args[0], "path.basename");
    fs::path path(p);
    
    std::string name = path.filename().string();
    if (name.empty() || name == "." || name == "..") {
        name = "";
    }
    
    return Value(name);
}

// path.dirname(path)
Value path_dirname(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.dirname expects 1 argument (path)");
    }
    
    std::string p = as_string(args[0], "path.dirname");
    fs::path path(p);
    
    fs::path parent = path.parent_path();
    if (parent.empty()) {
        parent = ".";
    }
    
    return Value(parent.string());
}

// path.extname(path)
Value path_extname(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.extname expects 1 argument (path)");
    }
    
    std::string p = as_string(args[0], "path.extname");
    fs::path path(p);
    
    std::string ext = path.extension().string();
    return Value(ext);
}

// path.isAbsolute(path)
Value path_is_absolute(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.isAbsolute expects 1 argument (path)");
    }
    
    std::string p = as_string(args[0], "path.isAbsolute");
    fs::path path(p);
    
    bool looks_unix_absolute = (!p.empty() && (p[0] == '/' || p[0] == '\\'));
    return Value(path.is_absolute() || looks_unix_absolute);
}

// 规范化路径
static fs::path normalize_path(const fs::path& path) {
    fs::path result;
    
    for (const auto& part : path) {
        if (part == ".") {
            continue;
        } else if (part == "..") {
            if (!result.empty()) {
                result = result.parent_path();
            }
        } else {
            result /= part;
        }
    }
    
    return result;
}

// path.normalize(path)
Value path_normalize(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.normalize expects 1 argument (path)");
    }
    
    std::string p = as_string(args[0], "path.normalize");
    fs::path path(p);
    
    fs::path normalized = normalize_path(path);
    return Value(normalized.string());
}

// path.resolve(paths)
Value path_resolve(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.resolve expects 1 argument (paths)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("path.resolve expects List<String>");
    }
    
    const auto& parts = *args[0].getArray();
    
    fs::path result = fs::current_path();
    
    for (const auto& part : parts) {
        if (part.getType() != Value::Type::String) {
            throw std::runtime_error("path.resolve expects List<String>");
        }
        
        fs::path p(part.getString());
        if (p.is_absolute()) {
            result = p;
        } else {
            result /= p;
        }
    }
    
    fs::path normalized = normalize_path(result);
    return Value(normalized.string());
}

// path.relative(from, to)
Value path_relative(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("path.relative expects 2 arguments (from, to)");
    }
    
    std::string from = as_string(args[0], "path.relative");
    std::string to = as_string(args[1], "path.relative");
    
    fs::path from_abs = normalize_path(fs::current_path() / from);
    fs::path to_abs = normalize_path(fs::current_path() / to);
    
    fs::path relative = fs::relative(to_abs, from_abs);
    return Value(relative.string());
}

// path.parse(path)
Value path_parse(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.parse expects 1 argument (path)");
    }
    
    std::string p = as_string(args[0], "path.parse");
    fs::path path(p);
    
    auto map = std::make_shared<std::unordered_map<std::string, Value>>();
    
    std::string root;
    if (path.has_root_name()) {
        root = path.root_name().string();
    }
    if (path.has_root_directory()) {
        root += path.root_directory().string();
    }
    (*map)["root"] = Value(root);
    
    fs::path parent = path.parent_path();
    (*map)["dir"] = Value(parent.empty() ? "" : parent.string());
    
    std::string filename = path.filename().string();
    (*map)["base"] = Value(filename);
    
    (*map)["ext"] = Value(path.extension().string());
    
    std::string stem = path.stem().string();
    (*map)["name"] = Value(stem);
    
    return Value::makeStruct(map);
}

// path.format(parts)
Value path_format(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("path.format expects 1 argument (parts)");
    }
    
    if (args[0].getType() != Value::Type::Struct) {
        throw std::runtime_error("path.format expects object struct");
    }
    
    auto obj = args[0].getStruct();
    
    auto get_str = [&](const std::string& key) -> std::string {
        auto it = obj->find(key);
        if (it != obj->end() && it->second.getType() == Value::Type::String) {
            return it->second.getString();
        }
        return "";
    };
    
    std::string dir = get_str("dir");
    std::string root = get_str("root");
    std::string base = get_str("base");
    
    if (base.empty()) {
        std::string name = get_str("name");
        std::string ext = get_str("ext");
        if (!ext.empty() && ext[0] != '.') {
            ext = "." + ext;
        }
        base = name + ext;
    }
    
    fs::path result;
    if (!dir.empty()) {
        result = fs::path(dir) / base;
    } else if (!root.empty()) {
        result = fs::path(root) / base;
    } else {
        result = fs::path(base);
    }
    
    return Value(result.string());
}

} // namespace path
} // namespace builtin
} // namespace rumina
