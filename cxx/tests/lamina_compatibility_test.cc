#include <test_framework.h>
#include <value.h>
#include <run_rumina.h>

using namespace rumina;
using namespace rumina::test;

void test_null_equality() {
    auto result = run_code("null == null;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Bool);
    assert_true(v.getBool());
}

void test_null_inequality() {
    auto result = run_code("null != null;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Bool);
    assert_false(v.getBool());
}

void test_null_vs_nonnull_equality() {
    auto result = run_code("null == 10;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Bool);
    assert_false(v.getBool());
}

void test_null_vs_nonnull_inequality() {
    auto result = run_code("null != 10;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Bool);
    assert_true(v.getBool());
}

void test_function_variable_call() {
    auto result = run_code(
        "func test() {"
        "    return 42;"
        "}"
        "var f = test;"
        "f();"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Int);
    assert_eq(v.getInt(), 42);
}

void test_function_in_struct_variable_call() {
    auto result = run_code(
        "func add(a, b) {"
        "    return a + b;"
        "}"
        "struct Ops {"
        "    add_fn = add;"
        "};"
        "var my_add = Ops.add_fn;"
        "my_add(10, 20);"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Int);
    assert_eq(v.getInt(), 30);
}

void test_complex_null_conditions() {
    auto result = run_code(
        "var a = null;"
        "var b = null;"
        "var c = 5;"
        "if (a == b) {"
        "    if (a != c) {"
        "        \"correct\";"
        "    } else { \"wrong\"; }"
        "} else { \"wrong\"; }"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::String);
    assert_eq(v.getString(), "correct");
}

int main() {
    TestRunner runner;
    
    runner.add_test("null_equality", test_null_equality);
    runner.add_test("null_inequality", test_null_inequality);
    runner.add_test("null_vs_nonnull_equality", test_null_vs_nonnull_equality);
    runner.add_test("null_vs_nonnull_inequality", test_null_vs_nonnull_inequality);
    runner.add_test("function_variable_call", test_function_variable_call);
    runner.add_test("function_in_struct_variable_call", test_function_in_struct_variable_call);
    runner.add_test("complex_null_conditions", test_complex_null_conditions);
    
    return runner.run_all();
}
