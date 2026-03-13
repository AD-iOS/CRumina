#include <builtin/builtin.h>
#include <compiler.h>
#include <lexer.h>
#include <parser.h>
#include <vm.h>
#include <value.h>
#include <bytecode_optimizer.h>
#include <optimizer.h>

namespace rumina {
namespace wasm {

// WASM 接口函數
extern "C" {
    const char* rumina_execute(const char* code) {
        static std::string result;
        
        try {
            // 詞法分析
            Lexer lexer(code);
            auto tokens = lexer.tokenize();
            
            // 語法分析
            Parser parser(tokens);
            auto statements = parser.parse();
            
            // AST优化
            ASTOptimizer ast_optimizer;
            auto opt_result = ast_optimizer.optimize(std::move(statements));
            if (opt_result.is_error()) {
                result = "AST optimization error: " + opt_result.error();
                return result.c_str();
            }
            
            // 編譯
            Compiler compiler;
            auto compile_result = compiler.compile(opt_result.value());
            if (compile_result.is_error()) {
                result = "Compilation error: " + compile_result.error();
                return result.c_str();
            }
            auto bytecode = std::move(compile_result.value());
            
            // 字节码优化
            BytecodeOptimizer bytecode_optimizer;
            bytecode_optimizer.optimize(bytecode);
            
            // 執行
            auto globals = std::make_shared<std::unordered_map<std::string, Value>>();
            builtin::register_builtins(*globals);
            
            VM vm(globals);
            vm.load(std::move(bytecode));
            auto run_result = vm.run();
            
            if (run_result.is_error()) {
                result = "Runtime error: " + run_result.error();
            } else if (run_result.value().has_value()) {
                result = run_result.value()->toString();
            } else {
                result = "";
            }
            
            return result.c_str();
            
        } catch (const std::exception& e) {
            result = "Exception: ";
            result += e.what();
            return result.c_str();
        }
    }
}

} // namespace wasm
} // namespace rumina
