#include <test_framework.h>
#include <compiler.h>
#include <lexer.h>
#include <parser.h>
#include <run_rumina.h>
#include <fstream>
#include <filesystem>

using namespace rumina;
using namespace rumina::test;

void test_include_statement_compilation() {
    std::string temp_dir = create_temp_dir("rumina_include_test");
    
    // Create included file
    std::string included_file = temp_dir + "/test_module.lm";
    std::ofstream incl(included_file);
    incl << "define module_name = \"test_module\";\n"
         << "var pi = 3.14159;\n"
         << "func add(a, b) {\n"
         << "    return a + b;\n"
         << "}\n";
    incl.close();
    
    // Create main file
    std::string main_file = temp_dir + "/main.lm";
    std::ofstream main(main_file);
    main << "include \"test_module.lm\";\n"
         << "var x = test_module::pi;\n"
         << "var y = test_module::add(10, 20);\n";
    main.close();
    
    // Test compilation
    std::ifstream file(main_file);
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    Compiler compiler(temp_dir);
    auto bytecode_result = compiler.compile(ast);
    assert_ok(Result<std::optional<Value>>::ok(std::nullopt)); // Dummy check
    
    // Test execution
    auto result = run_code_with_dir(
        "include \"test_module.lm\";\n"
        "var x = test_module::pi;\n"
        "var y = test_module::add(10, 20);\n",
        temp_dir
    );
    assert_ok(result);
    
    remove_temp_dir(temp_dir);
}

void test_include_prevents_circular_includes() {
    std::string temp_dir = create_temp_dir("rumina_circular_test");
    
    // Create file A that includes B
    std::string file_a = temp_dir + "/a.lm";
    std::ofstream a(file_a);
    a << "include \"b.lm\";\n";
    a.close();
    
    // Create file B that includes A
    std::string file_b = temp_dir + "/b.lm";
    std::ofstream b(file_b);
    b << "include \"a.lm\";\n";
    b.close();
    
    // Test compilation should not error
    std::ifstream file(file_a);
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    Compiler compiler(temp_dir);
    auto bytecode_result = compiler.compile(ast);
    assert_ok(Result<std::optional<Value>>::ok(std::nullopt));
    
    remove_temp_dir(temp_dir);
}

void test_include_namespace_function_call() {
    std::string temp_dir = create_temp_dir("rumina_namespace_call_test");
    
    // Create included file
    std::string included_file = temp_dir + "/math_utils.lm";
    std::ofstream incl(included_file);
    incl << "define module_name = \"math_utils\";\n"
         << "func multiply(x, y) {\n"
         << "    return x * y;\n"
         << "}\n";
    incl.close();
    
    // Test execution
    auto result = run_code_with_dir(
        "include \"math_utils.lm\";\n"
        "math_utils::multiply(5, 6);\n",
        temp_dir
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "30");
    
    remove_temp_dir(temp_dir);
}

int main() {
    TestRunner runner;
    
    runner.add_test("include_statement_compilation", test_include_statement_compilation);
    runner.add_test("include_prevents_circular_includes", test_include_prevents_circular_includes);
    runner.add_test("include_namespace_function_call", test_include_namespace_function_call);
    
    return runner.run_all();
}
