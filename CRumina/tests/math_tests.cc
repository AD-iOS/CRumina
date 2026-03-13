#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>

using namespace rumina;
using namespace rumina::test;

void test_sqrt_with_irrational() {
    auto result = run_code("sqrt(7 + 2*sqrt(10));");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    std::string value_str = value.value().toString();
    assert_true(value_str.find("√") != std::string::npos, 
                "Result should be in symbolic form with √");
}

void test_sqrt_basic() {
    auto result = run_code("sqrt(4);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "2");
}

void test_sqrt_non_perfect_square() {
    auto result = run_code("sqrt(2);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "√2");
}

void test_nested_sqrt() {
    auto result = run_code("sqrt(sqrt(16));");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "2");
}

void test_sqrt_with_multiplication() {
    auto result = run_code("sqrt(4 * 9);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "6");
}

void test_sqrt_negative() {
    auto result = run_code("sqrt(-4);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    std::string value_str = value.value().toString();
    assert_true(value_str.find("i") != std::string::npos || 
                value_str.find("2i") != std::string::npos);
}

void test_symbolic_multiplication() {
    auto result = run_code("2*sqrt(2);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "2√2");
}

void test_symbolic_addition() {
    auto result = run_code("sqrt(2) + sqrt(3);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    std::string value_str = value.value().toString();
    assert_true(value_str.find("√2") != std::string::npos && 
                value_str.find("√3") != std::string::npos);
}

void test_symbolic_sqrt_product() {
    auto result = run_code("sqrt(2) * sqrt(3);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "√6");
}

int main() {
    TestRunner runner;
    
    runner.add_test("sqrt_with_irrational", test_sqrt_with_irrational);
    runner.add_test("sqrt_basic", test_sqrt_basic);
    runner.add_test("sqrt_non_perfect_square", test_sqrt_non_perfect_square);
    runner.add_test("nested_sqrt", test_nested_sqrt);
    runner.add_test("sqrt_with_multiplication", test_sqrt_with_multiplication);
    runner.add_test("sqrt_negative", test_sqrt_negative);
    runner.add_test("symbolic_multiplication", test_symbolic_multiplication);
    runner.add_test("symbolic_addition", test_symbolic_addition);
    runner.add_test("symbolic_sqrt_product", test_symbolic_sqrt_product);
    
    return runner.run_all();
}
