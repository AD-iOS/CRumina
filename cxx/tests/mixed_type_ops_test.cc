#include <test_framework.h>
#include <interpreter.h>
#include <value.h>
#include <ast.h>

using namespace rumina;
using namespace rumina::test;

void test_int_float_mod() {
    Interpreter interpreter;
    Value int_val(static_cast<int64_t>(10));
    Value float_val(3.5);
    
    auto result = interpreter.eval_binary_op(int_val, BinOp::Mod, float_val);
    assert_ok(result);
    
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Float);
    assert_approx(3.0, value.getFloat(), 1e-10);
}

void test_int_float_comparison() {
    Interpreter interpreter;
    Value int_val(static_cast<int64_t>(1));
    Value float_val(1.0);
    
    auto result = interpreter.eval_binary_op(int_val, BinOp::Equal, float_val);
    assert_ok(result);
    
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());
}

void test_float_int_sub() {
    Interpreter interpreter;
    Value float_val(3.5);
    Value int_val(static_cast<int64_t>(1));
    
    auto result = interpreter.eval_binary_op(float_val, BinOp::Sub, int_val);
    assert_ok(result);
    
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Float);
    assert_approx(2.5, value.getFloat(), 1e-10);
}

void test_int_rational_mod() {
    Interpreter interpreter;
    Value int_val(static_cast<int64_t>(10));
    Value rational_val(BigRational(3, 1));
    
    auto result = interpreter.eval_binary_op(int_val, BinOp::Mod, rational_val);
    assert_ok(result);
    
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Rational);
    
    auto& r = value.getRational();
    BigRational expected(1, 1);
    assert_true(r == expected);
}

int main() {
    TestRunner runner;
    
    runner.add_test("int_float_mod", test_int_float_mod);
    runner.add_test("int_float_comparison", test_int_float_comparison);
    runner.add_test("float_int_sub", test_float_int_sub);
    runner.add_test("int_rational_mod", test_int_rational_mod);
    
    return runner.run_all();
}
