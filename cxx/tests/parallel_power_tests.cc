#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>
#include <chrono>

using namespace rumina;
using namespace rumina::test;

void test_large_power_computation() {
    auto result = run_code("var result = 100^10000; result;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::BigInt);
    
    // 100^10000 should have 20001 digits (10000 * 2 + 1)
    std::string digits = v.getBigInt().get_str();
    assert_true(digits.length() > 20000, "Expected > 20000 digits");
}

void test_threshold_behavior() {
    auto result = run_code("var result = 2^1000; result;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::BigInt);
    
    // 2^1000 has 302 digits
    std::string digits = v.getBigInt().get_str();
    assert_eq(digits.length(), 302);
}

void test_very_large_exponent() {
    auto result = run_code("var result = 2^100000; result;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::BigInt);
    
    // 2^100000 should have approximately 30103 digits (100000 * log10(2))
    std::string digits = v.getBigInt().get_str();
    assert_true(digits.length() > 30000 && digits.length() < 31000,
                "Expected ~30103 digits");
}

int main() {
    TestRunner runner;
    
    runner.add_test("large_power_computation", test_large_power_computation);
    runner.add_test("threshold_behavior", test_threshold_behavior);
    runner.add_test("very_large_exponent", test_very_large_exponent);
    
    return runner.run_all();
}
