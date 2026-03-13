#include <test_framework.h>
#include <value.h>
#include <run_rumina.h>

using namespace rumina;
using namespace rumina::test;

void test_decimal_function_converts_rational_to_float() {
    auto result = run_code("var x = 1/10; decimal(x);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(0.1, v.getFloat(), 1e-10);
}

void test_decimal_function_with_quarter() {
    auto result = run_code("var x = 1/4; decimal(x);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(0.25, v.getFloat(), 1e-10);
}

void test_decimal_function_with_third() {
    auto result = run_code("var x = 1/3; decimal(x);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(0.333333333333, v.getFloat(), 1e-10);
}

void test_decimal_literal_converted_to_rational() {
    auto result = run_code("var x = 0.1; typeof(x);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::String);
    assert_eq(v.getString(), "rational");
}

void test_float_function_converts_rational() {
    auto result = run_code("var x = 3/4; var y = float(x); y;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(0.75, v.getFloat(), 1e-10);
}

void test_rational_arithmetic_stays_exact() {
    auto result = run_code("var x = 0.1; var y = 0.2; var z = x + y; z;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    std::string display = value.value().toString();
    assert_eq(display, "3/10");
}

void test_decimal_maintains_precision_comparison() {
    auto result = run_code("0.1 + 0.2 == 0.3;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Bool);
    assert_true(v.getBool());
}

void test_mixed_rational_and_decimal_conversion() {
    auto result = run_code("var r = 0.1 + 0.2; var d = decimal(r); var f = float(d); f;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(0.3, v.getFloat(), 1e-10);
}

void test_typeof_after_conversion() {
    auto result = run_code(
        "var x = 1/2;"
        "var t1 = typeof(x);"
        "var y = decimal(x);"
        "var t2 = typeof(y);"
        "t1 + \",\" + t2;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::String);
    assert_eq(v.getString(), "rational,float");
}

void test_complex_rational_expression() {
    auto result = run_code("(0.1 + 0.2) * (0.5 + 0.5);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    std::string display = value.value().toString();
    assert_eq(display, "3/10");
}

int main() {
    TestRunner runner;
    
    runner.add_test("decimal_function_converts_rational_to_float", 
                    test_decimal_function_converts_rational_to_float);
    runner.add_test("decimal_function_with_quarter", test_decimal_function_with_quarter);
    runner.add_test("decimal_function_with_third", test_decimal_function_with_third);
    runner.add_test("decimal_literal_converted_to_rational", 
                    test_decimal_literal_converted_to_rational);
    runner.add_test("float_function_converts_rational", test_float_function_converts_rational);
    runner.add_test("rational_arithmetic_stays_exact", test_rational_arithmetic_stays_exact);
    runner.add_test("decimal_maintains_precision_comparison", 
                    test_decimal_maintains_precision_comparison);
    runner.add_test("mixed_rational_and_decimal_conversion", 
                    test_mixed_rational_and_decimal_conversion);
    runner.add_test("typeof_after_conversion", test_typeof_after_conversion);
    runner.add_test("complex_rational_expression", test_complex_rational_expression);
    
    return runner.run_all();
}
