#include <interpreter.h>
#include <compiler.h>
#include <vm.h>
#include <lexer.h>
#include <parser.h>
#include <bytecode_optimizer.h>
#include <optimizer.h>
#include <builtin/process.h>

#include <iostream>
#include <fstream>
#include <string>
#include <csignal>
#include <thread>
#include <filesystem>

namespace rumina {

// 增大栈大小以处理深层递归
constexpr size_t STACK_SIZE = 128 * 1024 * 1024; // 128 MB

void check_semicolons(const std::string& contents, const std::string& filename) {
    std::istringstream iss(contents);
    std::string line;
    int line_num = 0;
    bool in_multiline_comment = false;
    
    while (std::getline(iss, line)) {
        line_num++;
        
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            continue;
        }
        std::string trimmed = line.substr(start);
        
        if (trimmed.find("/*") != std::string::npos) {
            in_multiline_comment = true;
        }
        if (trimmed.find("*/") != std::string::npos) {
            in_multiline_comment = false;
            continue;
        }
        
        if (in_multiline_comment) {
            continue;
        }
        
        if (trimmed.empty() ||
            trimmed.rfind("//", 0) == 0 ||
            trimmed.rfind("*", 0) == 0 ||
            trimmed.rfind("if ", 0) == 0 ||
            trimmed.rfind("while ", 0) == 0 ||
            trimmed.rfind("loop ", 0) == 0 ||
            trimmed.rfind("func ", 0) == 0 ||
            trimmed.rfind("include ", 0) == 0 ||
            trimmed.back() == '{' ||
            trimmed.back() == '}' ||
            trimmed == "}") {
            continue;
        }
        
        size_t comment_pos = trimmed.find("//");
        std::string code_part = (comment_pos != std::string::npos) 
            ? trimmed.substr(0, comment_pos) 
            : trimmed;
        
        size_t last_non_space = code_part.find_last_not_of(" \t");
        if (last_non_space != std::string::npos) {
            code_part = code_part.substr(0, last_non_space + 1);
        }
        
        if (!code_part.empty() && code_part.back() != ';' && code_part.back() != '{') {
            std::cerr << "Warning: " << filename << ":" << line_num 
                      << ": Statement should end with ';'\n";
        }
    }
}

int run_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error reading file '" << filename << "'\n";
        return 1;
    }
    
    std::string contents((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    
    check_semicolons(contents, filename);
    
    std::filesystem::path file_path(filename);
    std::string file_dir = file_path.parent_path().string();
    
    try {
        Lexer lexer(contents);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto statements = parser.parse();
        
        ASTOptimizer ast_optimizer;
        auto optimized_result = ast_optimizer.optimize(std::move(statements));
        if (optimized_result.is_error()) {
            std::cerr << "AST optimization error: " << optimized_result.error() << "\n";
            return 1;
        }
        
        Compiler compiler(file_dir);
        auto compile_result = compiler.compile(optimized_result.value());
        if (compile_result.is_error()) {
            std::cerr << "Compilation error: " << compile_result.error() << "\n";
            return 1;
        }
        auto bytecode = std::move(compile_result.value());
        
        BytecodeOptimizer bytecode_optimizer;
        bytecode_optimizer.optimize(bytecode);
        
        Interpreter interpreter;
        auto globals = interpreter.getGlobals();
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

void run_repl() {
    std::cout << "Rumina\n";
    std::cout << "Type 'exit' to quit, or enter Lamina code to execute.\n\n";
    
    Interpreter interpreter;
    auto globals = interpreter.getGlobals();
    
    int line_number = 1;
    
    while (true) {
        std::cout << "rumina [" << line_number << "]> ";
        std::cout.flush();
        
        std::string input;
        if (!std::getline(std::cin, input)) {
            std::cout << "\n";
            break;
        }
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        bool needs_semicolon = !input.empty() && input.back() != ';' &&
            input.rfind("if ", 0) != 0 &&
            input.rfind("while ", 0) != 0 &&
            input.rfind("loop ", 0) != 0 &&
            input.rfind("func ", 0) != 0 &&
            input.back() != '}';
        
        if (needs_semicolon) {
            std::cerr << "Warning: Statement should end with ';'\n";
            input += ";";
        }
        
        try {
            Lexer lexer(input);
            auto tokens = lexer.tokenize();
            
            Parser parser(tokens);
            auto statements = parser.parse();
            
            Compiler compiler;
            auto compile_result = compiler.compile(statements);
            if (compile_result.is_error()) {
                std::cerr << "Compilation error: " << compile_result.error() << "\n";
                continue;
            }
            auto bytecode = std::move(compile_result.value());
            
            VM vm(globals);
            vm.load(std::move(bytecode));
            
            auto result = vm.run();
            if (result.is_error()) {
                std::cerr << "Runtime error: " << result.error() << "\n";
                continue;
            }
            
            auto value_opt = result.value();
            if (value_opt.has_value() && value_opt->getType() != Value::Type::Null) {
                std::cout << value_opt->toString() << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
        
        line_number++;
    }
    
    std::cout << "Goodbye!\n";
}

} // namespace rumina

int main(int argc, char* argv[]) {
    // 初始化 process 模块的命令行参数
    rumina::builtin::process::init_process_args(argc, argv);
    
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg.size() >= 3 && arg.substr(arg.size() - 3) == ".lm") {
            return rumina::run_file(arg);
        } else {
            std::cerr << "Error: No .lm file specified\n";
            std::cerr << "Usage:\n";
            std::cerr << "  rumina              - Start REPL\n";
            std::cerr << "  rumina <file.lm>    - Run Lamina file\n";
            return 1;
        }
    } else {
        rumina::run_repl();
        return 0;
    }
}
