#include <builtin/env.h>

#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char** environ;
#endif
#endif

namespace rumina {
namespace builtin {
namespace env {

// 创建环境变量模块
Value create_env_module() {
    auto ns = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*ns)["get"] = Value::makeNativeFunction("env::get", env_get);
    (*ns)["set"] = Value::makeNativeFunction("env::set", env_set);
    (*ns)["has"] = Value::makeNativeFunction("env::has", env_has);
    (*ns)["remove"] = Value::makeNativeFunction("env::remove", env_remove);
    (*ns)["all"] = Value::makeNativeFunction("env::all", env_all);
    (*ns)["keys"] = Value::makeNativeFunction("env::keys", env_keys);
    
    return Value::makeModule(ns);
}

// 获取字符串参数
static std::string get_str(const Value& v, const std::string& fn_name) {
    if (v.getType() != Value::Type::String) {
        throw std::runtime_error(fn_name + " expects string");
    }
    return v.getString();
}

// env.get(key)
Value env_get(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("env.get expects 1 argument (key)");
    }
    
    std::string key = get_str(args[0], "env.get");
    
    const char* value = std::getenv(key.c_str());
    if (value) {
        return Value(std::string(value));
    } else {
        return Value();
    }
}

// env.set(key, value)
Value env_set(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("env.set expects 2 arguments (key, value)");
    }
    
    std::string key = get_str(args[0], "env.set");
    std::string value = get_str(args[1], "env.set");
    
#ifdef _WIN32
    _putenv_s(key.c_str(), value.c_str());
#else
    setenv(key.c_str(), value.c_str(), 1);
#endif
    
    return Value();
}

// env.has(key)
Value env_has(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("env.has expects 1 argument (key)");
    }
    
    std::string key = get_str(args[0], "env.has");
    
    const char* value = std::getenv(key.c_str());
    return Value(value != nullptr);
}

// env.remove(key)
Value env_remove(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("env.remove expects 1 argument (key)");
    }
    
    std::string key = get_str(args[0], "env.remove");
    
#ifdef _WIN32
    _putenv_s(key.c_str(), "");
#else
    unsetenv(key.c_str());
#endif
    
    return Value();
}

// env.all()
Value env_all(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("env.all expects no arguments");
    }
    
    auto map = std::make_shared<std::unordered_map<std::string, Value>>();
    
#ifdef _WIN32
    char* env = GetEnvironmentStrings();
    if (env) {
        for (char* var = env; *var; var += strlen(var) + 1) {
            std::string entry(var);
            size_t eq_pos = entry.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = entry.substr(0, eq_pos);
                std::string value = entry.substr(eq_pos + 1);
                (*map)[key] = Value(value);
            }
        }
        FreeEnvironmentStrings(env);
    }
#else
    extern char** environ;
    for (char** env = environ; *env; ++env) {
        std::string entry(*env);
        size_t eq_pos = entry.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = entry.substr(0, eq_pos);
            std::string value = entry.substr(eq_pos + 1);
            (*map)[key] = Value(value);
        }
    }
#endif
    
    return Value::makeStruct(map);
}

// env.keys()
Value env_keys(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("env.keys expects no arguments");
    }
    
    std::vector<Value> keys;
    
#ifdef _WIN32
    char* env = GetEnvironmentStrings();
    if (env) {
        for (char* var = env; *var; var += strlen(var) + 1) {
            std::string entry(var);
            size_t eq_pos = entry.find('=');
            if (eq_pos != std::string::npos) {
                keys.push_back(Value(entry.substr(0, eq_pos)));
            }
        }
        FreeEnvironmentStrings(env);
    }
#else
    extern char** environ;
    for (char** env = environ; *env; ++env) {
        std::string entry(*env);
        size_t eq_pos = entry.find('=');
        if (eq_pos != std::string::npos) {
            keys.push_back(Value(entry.substr(0, eq_pos)));
        }
    }
#endif
    
    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(keys)));
}

} // namespace env
} // namespace builtin
} // namespace rumina
