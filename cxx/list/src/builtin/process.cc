#include <builtin/process.h>

#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <shellapi.h>
#else
#include <unistd.h>
#include <limits.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif

namespace rumina {
namespace builtin {
namespace process {

namespace fs = std::filesystem;

static std::vector<std::string> g_argv;
static bool g_argv_initialized = false;

void init_process_args(int argc, char* argv[]) {
    g_argv.clear();
    for (int i = 0; i < argc; ++i) {
        g_argv.push_back(std::string(argv[i]));
    }
    g_argv_initialized = true;
}

Value create_process_module() {
    auto ns = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*ns)["args"] = Value::makeNativeFunction("process::args", process_args);
    (*ns)["cwd"] = Value::makeNativeFunction("process::cwd", process_cwd);
    (*ns)["setCwd"] = Value::makeNativeFunction("process::setCwd", process_set_cwd);
    (*ns)["pid"] = Value::makeNativeFunction("process::pid", process_pid);
    (*ns)["exit"] = Value::makeNativeFunction("process::exit", process_exit);
    (*ns)["platform"] = Value::makeNativeFunction("process::platform", process_platform);
    (*ns)["arch"] = Value::makeNativeFunction("process::arch", process_arch);
    (*ns)["version"] = Value::makeNativeFunction("process::version", process_version);
    (*ns)["execPath"] = Value::makeNativeFunction("process::execPath", process_exec_path);
    
    return Value::makeModule(ns);
}

static std::string as_string(const Value& v, const std::string& fn_name) {
    if (v.getType() != Value::Type::String) {
        throw std::runtime_error(fn_name + " expects string");
    }
    return v.getString();
}

Value process_args(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("process.args expects no arguments");
    }
    
    std::vector<Value> result;
    
#ifdef _WIN32
    LPWSTR cmd_line = GetCommandLineW();
    int argc;
    LPWSTR* wargv = CommandLineToArgvW(cmd_line, &argc);
    if (wargv) {
        for (int i = 0; i < argc; ++i) {
            int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
            std::string arg(len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &arg[0], len, nullptr, nullptr);
            arg.pop_back();
            result.push_back(Value(arg));
        }
        LocalFree(wargv);
    }
#else
    if (g_argv_initialized) {
        for (const auto& arg : g_argv) {
            result.push_back(Value(arg));
        }
    } else {
        std::ifstream cmdline_file("/proc/self/cmdline", std::ios::binary);
        if (cmdline_file.is_open()) {
            std::string content;
            std::getline(cmdline_file, content, '\0');
            while (!content.empty()) {
                result.push_back(Value(content));
                std::getline(cmdline_file, content, '\0');
            }
        }
    }
#endif
    
    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(result)));
}

Value process_cwd(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("process.cwd expects no arguments");
    }
    
    std::error_code ec;
    fs::path cwd = fs::current_path(ec);
    if (ec) {
        throw std::runtime_error("process.cwd failed: " + ec.message());
    }
    
    return Value(cwd.string());
}

Value process_set_cwd(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("process.setCwd expects 1 argument (path)");
    }
    
    std::string path = as_string(args[0], "process.setCwd");
    
    std::error_code ec;
    fs::current_path(path, ec);
    if (ec) {
        throw std::runtime_error("process.setCwd failed for '" + path + "': " + ec.message());
    }
    
    return Value();
}

Value process_pid(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("process.pid expects no arguments");
    }
    
#ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif
    
    return Value(static_cast<int64_t>(pid));
}

Value process_exit(const std::vector<Value>& args) {
    int code = 0;
    if (args.size() == 1) {
        code = static_cast<int>(args[0].toInt());
    } else if (!args.empty()) {
        throw std::runtime_error("process.exit expects 0 or 1 arguments");
    }
    
    std::exit(code);
    return Value();
}

Value process_platform(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("process.platform expects no arguments");
    }
    
#ifdef _WIN32
    return Value("win32");
#elif __APPLE__
    return Value("darwin");
#elif __linux__
    return Value("linux");
#elif __FreeBSD__
    return Value("freebsd");
#elif __OpenBSD__
    return Value("openbsd");
#elif __NetBSD__
    return Value("netbsd");
#elif __sun
    return Value("sunos");
#else
    return Value("unknown");
#endif
}

Value process_arch(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("process.arch expects no arguments");
    }
    
#if defined(__x86_64__) || defined(_M_X64)
    return Value("x64");
#elif defined(__i386__) || defined(_M_IX86)
    return Value("ia32");
#elif defined(__aarch64__) || defined(_M_ARM64)
    return Value("arm64");
#elif defined(__arm__) || defined(_M_ARM)
    return Value("arm");
#elif defined(__mips__)
    return Value("mips");
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    return Value("ppc");
#elif defined(__s390x__)
    return Value("s390x");
#elif defined(__riscv)
    return Value("riscv64");
#else
    return Value("unknown");
#endif
}

Value process_version(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("process.version expects no arguments");
    }
    
#ifdef RUMINA_VERSION
    return Value("v" + std::string(RUMINA_VERSION));
#else
    return Value("v1.0.0");
#endif
}

Value process_exec_path(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("process.execPath expects no arguments");
    }
    
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        int mb_len = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
        std::string result(mb_len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], mb_len, nullptr, nullptr);
        result.pop_back();
        return Value(result);
    }
#elif defined(__APPLE__)
    uint32_t bufsize = 0;
    _NSGetExecutablePath(nullptr, &bufsize);
    if (bufsize > 0) {
        std::vector<char> path(bufsize);
        if (_NSGetExecutablePath(path.data(), &bufsize) == 0) {
            char resolved[PATH_MAX];
            if (realpath(path.data(), resolved) != nullptr) {
                return Value(std::string(resolved));
            }
            return Value(std::string(path.data()));
        }
    }
#elif defined(__linux__)
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return Value(std::string(path));
    }
#endif
    
    return Value();
}

} // namespace process
} // namespace builtin
} // namespace rumina
