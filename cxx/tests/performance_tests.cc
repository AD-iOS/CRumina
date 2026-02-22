#include <test_framework.h>
#include <compiler.h>
#include <interpreter.h>
#include <lexer.h>
#include <parser.h>
#include <vm.h>
#include <chrono>

using namespace rumina;
using namespace rumina::test;

const char* FIB_CODE = R"(
func fib(n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}
fib(20);
)";

const char* ARITHMETIC_CODE = R"(
var sum = 0;
var i = 0;
while (i < 1000) {
    sum = sum + i;
    i = i + 1;
}
sum;
)";

void test_vm_performance_fibonacci() {
    // VM test
    auto vm_start = std::chrono::high_resolution_clock::now();
    
    Lexer vm_lexer(FIB_CODE);
    auto vm_tokens = vm_lexer.tokenize();
    Parser vm_parser(vm_tokens);
    auto vm_ast = vm_parser.parse();
    Compiler vm_compiler;
    auto vm_bytecode_result = vm_compiler.compile(vm_ast);
    assert_ok(Result<std::optional<Value>>::ok(std::nullopt));
    
    Interpreter vm_interp;
    auto vm_globals = vm_interp.getGlobals();
    VM vm(vm_globals);
    vm.load(vm_bytecode_result.value());
    auto vm_result = vm.run();
    
    auto vm_end = std::chrono::high_resolution_clock::now();
    auto vm_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        vm_end - vm_start).count();
    
    // Interpreter test
    auto interp_start = std::chrono::high_resolution_clock::now();
    
    Lexer interp_lexer(FIB_CODE);
    auto interp_tokens = interp_lexer.tokenize();
    Parser interp_parser(interp_tokens);
    auto interp_ast = interp_parser.parse();
    Interpreter interp;
    auto interp_result = interp.interpret(std::move(interp_ast));
    
    auto interp_end = std::chrono::high_resolution_clock::now();
    auto interp_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        interp_end - interp_start).count();
    
    // Verify results match
    assert_ok(vm_result);
    assert_ok(interp_result);
    
    auto vm_val = vm_result.value();
    auto interp_val = interp_result.value();
    
    assert_true(vm_val.has_value());
    assert_true(interp_val.has_value());
    
    auto& vm_v = vm_val.value();
    auto& interp_v = interp_val.value();
    
    assert_eq(vm_v.getType(), Value::Type::Int);
    assert_eq(interp_v.getType(), Value::Type::Int);
    assert_eq(vm_v.getInt(), interp_v.getInt());
    assert_eq(vm_v.getInt(), 6765);
    
    std::cout << "VM time: " << vm_time << "ms, "
              << "Interpreter time: " << interp_time << "ms\n";
}

void test_vm_arithmetic_performance() {
    // VM test
    auto vm_start = std::chrono::high_resolution_clock::now();
    
    Lexer vm_lexer(ARITHMETIC_CODE);
    auto vm_tokens = vm_lexer.tokenize();
    Parser vm_parser(vm_tokens);
    auto vm_ast = vm_parser.parse();
    Compiler vm_compiler;
    auto vm_bytecode_result = vm_compiler.compile(vm_ast);
    assert_ok(Result<std::optional<Value>>::ok(std::nullopt));
    
    Interpreter vm_interp;
    auto vm_globals = vm_interp.getGlobals();
    VM vm(vm_globals);
    vm.load(vm_bytecode_result.value());
    auto vm_result = vm.run();
    
    auto vm_end = std::chrono::high_resolution_clock::now();
    auto vm_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        vm_end - vm_start).count();
    
    // Interpreter test
    auto interp_start = std::chrono::high_resolution_clock::now();
    
    Lexer interp_lexer(ARITHMETIC_CODE);
    auto interp_tokens = interp_lexer.tokenize();
    Parser interp_parser(interp_tokens);
    auto interp_ast = interp_parser.parse();
    Interpreter interp;
    auto interp_result = interp.interpret(std::move(interp_ast));
    
    auto interp_end = std::chrono::high_resolution_clock::now();
    auto interp_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        interp_end - interp_start).count();
    
    // Verify results match
    assert_ok(vm_result);
    assert_ok(interp_result);
    
    auto vm_val = vm_result.value();
    auto interp_val = interp_result.value();
    
    assert_true(vm_val.has_value());
    assert_true(interp_val.has_value());
    
    auto& vm_v = vm_val.value();
    auto& interp_v = interp_val.value();
    
    assert_eq(vm_v.getType(), Value::Type::Int);
    assert_eq(interp_v.getType(), Value::Type::Int);
    assert_eq(vm_v.getInt(), interp_v.getInt());
    assert_eq(vm_v.getInt(), 499500);
    
    std::cout << "Arithmetic: VM time: " << vm_time << "ms, "
              << "Interpreter time: " << interp_time << "ms\n";
}

int main() {
    TestRunner runner;
    
    runner.add_test("vm_performance_fibonacci", test_vm_performance_fibonacci);
    runner.add_test("vm_arithmetic_performance", test_vm_arithmetic_performance);
    
    return runner.run_all();
}
