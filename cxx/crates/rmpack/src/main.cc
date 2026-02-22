#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

#include <lexer.h>
#include <parser.h>
#include <compiler.h>
#include <vm.h>
#include <interpreter.h>
#include <builtin/builtin.h>
#include <optimizer.h>
#include <bytecode_optimizer.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif

namespace rumina {

constexpr uint8_t MAGIC[] = {0x52, 0x4D, 0x50, 0x4B, 0x53, 0x52, 0x43, 0x00}; // "RMPKSRC\0"

struct PackageConfig {
    std::string input_file;
    std::string output_file;
    bool optimize = true;
    bool debug_info = false;
};

int run_embedded_source(const std::string& source) {
    try {
        // 詞法分析
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        // 語法分析
        Parser parser(tokens);
        auto statements = parser.parse();
        
        // AST優化
        ASTOptimizer ast_optimizer;
        auto opt_result = ast_optimizer.optimize(std::move(statements));
        if (opt_result.is_error()) {
            std::cerr << "AST optimization error: " << opt_result.error() << "\n";
            return 1;
        }
        
        // 編譯為字節碼
        Compiler compiler;
        auto compile_result = compiler.compile(opt_result.value());
        if (compile_result.is_error()) {
            std::cerr << "Compilation error: " << compile_result.error() << "\n";
            return 1;
        }
        auto bytecode = std::move(compile_result.value());
        
        // 字節碼優化
        BytecodeOptimizer bytecode_optimizer;
        bytecode_optimizer.optimize(bytecode);
        
        // 創建全局環境
        auto globals = std::make_shared<std::unordered_map<std::string, Value>>();
        builtin::register_builtins(*globals);
        
        // 執行
        VM vm(globals);
        vm.load(std::move(bytecode));
        auto result = vm.run();
        
        if (result.is_error()) {
            std::cerr << "Runtime error: " << result.error() << "\n";
            return 1;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

std::string extract_embedded_source(const std::string& exe_path) {
    std::ifstream file(exe_path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::vector<char> content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    
    // 從文件末尾查找魔術標記
    for (size_t i = content.size(); i >= sizeof(MAGIC); i--) {
        if (memcmp(&content[i - sizeof(MAGIC)], MAGIC, sizeof(MAGIC)) == 0) {
            // 找到魔術標記，讀取長度
            uint64_t length;
            memcpy(&length, &content[i], sizeof(length));
            
            size_t source_start = i + sizeof(length);
            if (source_start + length <= content.size()) {
                return std::string(content.begin() + source_start,
                                  content.begin() + source_start + length);
            }
        }
    }
    return "";
}

void print_usage() {
    std::cout << "Rumina Packager - Package .lm files into standalone executables\n";
    std::cout << "\n";
    std::cout << "Usage:\n";
    std::cout << "  rmpack <input.lm> [output]\n";
    std::cout << "\n";
    std::cout << "Arguments:\n";
    std::cout << "  <input.lm>   Input Lamina source file\n";
    std::cout << "  [output]     Output executable name (optional)\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  --no-optimize   Disable optimization\n";
    std::cout << "  --debug         Include debug information\n";
    std::cout << "  --help, -h      Show this help message\n";
}

std::vector<char> read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    return std::vector<char>(std::istreambuf_iterator<char>(file),
                              std::istreambuf_iterator<char>());
}

void write_file(const std::string& filename, const std::vector<char>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write file: " + filename);
    }
    file.write(data.data(), data.size());
}

std::string get_self_path() {
    std::string exe_path;
    char self_path[1024];
    
#ifdef _WIN32
    GetModuleFileNameA(NULL, self_path, sizeof(self_path));
    exe_path = self_path;
#elif defined(__APPLE__)
    uint32_t bufsize = sizeof(self_path);
    if (_NSGetExecutablePath(self_path, &bufsize) == 0) {
        char resolved[PATH_MAX];
        if (realpath(self_path, resolved) != nullptr) {
            exe_path = resolved;
        } else {
            exe_path = self_path;
        }
    } else {
        throw std::runtime_error("Cannot get executable path");
    }
#else
    ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (len != -1) {
        self_path[len] = '\0';
        exe_path = self_path;
    } else {
        throw std::runtime_error("Cannot get executable path");
    }
#endif
    
    return exe_path;
}

#ifdef _WIN32
void set_executable_permission(const std::string&) {}
#else
void set_executable_permission(const std::string& filename) {
    chmod(filename.c_str(), 0755);
}
#endif

int package(const PackageConfig& config) {
    std::cout << "Reading " << config.input_file << " ...\n";
    
    auto source_data = read_file(config.input_file);
    std::string source(source_data.begin(), source_data.end());
    
    std::cout << "Source code size: " << source.size() << " bytes\n";
    std::cout << "Generating executable " << config.output_file << " ...\n";
    
    std::string exe_path = get_self_path();
    auto rmpack_binary = read_file(exe_path);
    
    rmpack_binary.insert(rmpack_binary.end(), MAGIC, MAGIC + sizeof(MAGIC));
    
    uint64_t length = source.size();
    const char* length_bytes = reinterpret_cast<const char*>(&length);
    rmpack_binary.insert(rmpack_binary.end(), length_bytes, length_bytes + sizeof(length));
    
    rmpack_binary.insert(rmpack_binary.end(), source.begin(), source.end());
    
    write_file(config.output_file, rmpack_binary);
    set_executable_permission(config.output_file);
    
    std::cout << "✓ Packaging completed successfully!\n";
    return 0;
}

} // namespace rumina

int main(int argc, char* argv[]) {
    // 檢查是否有嵌入的源代碼
    std::string self_path = argv[0];
    std::string embedded_source = rumina::extract_embedded_source(self_path);
    
    if (!embedded_source.empty()) {
        // 有嵌入的源代碼，執行它
        return rumina::run_embedded_source(embedded_source);
    }
    
    // 否則作為打包器運行
    if (argc < 2) {
        rumina::print_usage();
        return 1;
    }
    
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    
    if (args[0] == "--help" || args[0] == "-h") {
        rumina::print_usage();
        return 0;
    }
    
    rumina::PackageConfig config;
    bool has_input = false;
    
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--no-optimize") {
            config.optimize = false;
        } else if (args[i] == "--debug") {
            config.debug_info = true;
        } else if (args[i][0] == '-') {
            std::cerr << "Error: Unknown option '" << args[i] << "'\n";
            std::cerr << "Use --help for usage information\n";
            return 1;
        } else {
            if (!has_input) {
                config.input_file = args[i];
                has_input = true;
            } else if (config.output_file.empty()) {
                config.output_file = args[i];
            } else {
                std::cerr << "Error: Too many arguments\n";
                return 1;
            }
        }
    }
    
    if (!has_input) {
        std::cerr << "Error: No input file specified\n";
        return 1;
    }
    
    if (config.output_file.empty()) {
        config.output_file = config.input_file;
        size_t dot_pos = config.output_file.rfind('.');
        if (dot_pos != std::string::npos) {
            config.output_file = config.output_file.substr(0, dot_pos);
        }
#ifdef _WIN32
        config.output_file += ".exe";
#endif
    }
    
    try {
        return rumina::package(config);
    } catch (const std::exception& e) {
        std::cerr << "Packaging failed: " << e.what() << "\n";
        return 1;
    }
}
