#include <compiler.h>
#include <lexer.h>
#include <parser.h>
#include <bytecode_optimizer.h>
#include <optimizer.h>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace rumina {

int compile_file(const std::string& input_file, const std::string& output_file) {
    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error reading file '" << input_file << "'\n";
        return 1;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    std::filesystem::path input_path(input_file);
    std::string input_dir = input_path.parent_path().string();
    
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        Parser parser(tokens);
        auto statements = parser.parse();
        
        ASTOptimizer ast_optimizer;
        auto opt_result = ast_optimizer.optimize(std::move(statements));
        if (opt_result.is_error()) {
            std::cerr << "AST optimization error: " << opt_result.error() << "\n";
            return 1;
        }
        
        Compiler compiler(input_dir);
        auto compile_result = compiler.compile(opt_result.value());
        if (compile_result.is_error()) {
            std::cerr << "Compilation error: " << compile_result.error() << "\n";
            return 1;
        }
        auto bytecode = std::move(compile_result.value());
        
        BytecodeOptimizer bytecode_optimizer;
        bytecode_optimizer.optimize(bytecode);
        
        std::string bytecode_text = bytecode.serialize();
        
        std::ofstream out(output_file);
        if (!out.is_open()) {
            std::cerr << "Error writing to '" << output_file << "'\n";
            return 1;
        }
        out << bytecode_text;
        
        std::cout << "Successfully compiled '" << input_file << "' to '" << output_file << "'\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace rumina

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ruminac <input.lm> [output.rmc]\n";
        std::cerr << "  Compiles a .lm file to .rmc bytecode\n";
        std::cerr << "\n";
        std::cerr << "Examples:\n";
        std::cerr << "  ruminac test.lm           # Creates test.rmc\n";
        std::cerr << "  ruminac test.lm out.rmc   # Creates out.rmc\n";
        return 1;
    }
    
    std::string input_file = argv[1];
    
    std::string output_file;
    if (argc >= 3) {
        output_file = argv[2];
    } else {
        std::filesystem::path input_path(input_file);
        std::string stem = input_path.stem().string();
        output_file = stem + ".rmc";
    }
    
    return rumina::compile_file(input_file, output_file);
}
