#include <test_framework.h>
#include <value.h>
#include <run_rumina.h>
#include <num/big_rational.h>

using namespace rumina;
using namespace rumina::test;

void test_decimal_addition_precision() {
    auto result = run_code("0.1 + 0.2;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(3, 10);
    assert_true(r == expected, "0.1 + 0.2 should equal 3/10");
}

void test_decimal_equality() {
    auto result = run_code("0.1 + 0.2 == 0.3;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Bool);
    assert_true(v.getBool(), "0.1 + 0.2 should equal 0.3");
}

void test_simple_decimal_parsing() {
    auto result = run_code("0.1;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(1, 10);
    assert_true(r == expected, "0.1 should equal 1/10");
}

void test_decimal_quarter() {
    auto result = run_code("0.25;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(1, 4);
    assert_true(r == expected, "0.25 should equal 1/4");
}

void test_decimal_multiplication() {
    auto result = run_code("0.5 * 0.5;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(1, 4);
    assert_true(r == expected, "0.5 * 0.5 should equal 1/4");
}

void test_decimal_subtraction() {
    auto result = run_code("0.3 - 0.1;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(1, 5);
    assert_true(r == expected, "0.3 - 0.1 should equal 1/5");
}

void test_decimal_display() {
    auto result = run_code("0.1;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    std::string display = value.value().toString();
    assert_eq(display, "1/10", "0.1 should display as 1/10");
}

void test_multiple_decimal_places() {
    auto result = run_code("0.125;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(1, 8);
    assert_true(r == expected, "0.125 should equal 1/8");
}

void test_decimal_with_integer_part() {
    auto result = run_code("1.5;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(3, 2);
    assert_true(r == expected, "1.5 should equal 3/2");
}

void test_complex_decimal_expression() {
    auto result = run_code("(0.1 + 0.2) * 2;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(3, 5);
    assert_true(r == expected, "(0.1 + 0.2) * 2 should equal 3/5");
}

void test_negative_decimal() {
    auto result = run_code("-0.1;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(-1, 10);
    assert_true(r == expected, "-0.1 should equal -1/10");
}

void test_zero_decimal() {
    auto result = run_code("0.0;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Rational);
    
    auto& r = v.getRational();
    BigRational expected(0, 1);
    assert_true(r == expected, "0.0 should equal 0/1");
}

int main() {
    TestRunner runner;
    
    runner.add_test("decimal_addition_precision", test_decimal_addition_precision);
    runner.add_test("decimal_equality", test_decimal_equality);
    runner.add_test("simple_decimal_parsing", test_simple_decimal_parsing);
    runner.add_test("decimal_quarter", test_decimal_quarter);
    runner.add_test("decimal_multiplication", test_decimal_multiplication);
    runner.add_test("decimal_subtraction", test_decimal_subtraction);
    runner.add_test("decimal_display", test_decimal_display);
    runner.add_test("multiple_decimal_places", test_multiple_decimal_places);
    runner.add_test("decimal_with_integer_part", test_decimal_with_integer_part);
    runner.add_test("complex_decimal_expression", test_complex_decimal_expression);
    runner.add_test("negative_decimal", test_negative_decimal);
    runner.add_test("zero_decimal", test_zero_decimal);
    
    return runner.run_all();
}
