#include <test_framework.h>
#include <value.h>
#include <run_rumina.h>

using namespace rumina;
using namespace rumina::test;

void test_let_is_immutable() {
    auto result = run_code("let x = 1; x = 2;");
    assert_error(result, "reassigning let should error");
}

void test_let_member_assign_is_immutable() {
    auto result = run_code("let s = null; s.a = 1;");
    assert_error(result, "member assignment on let binding should error");
}

void test_pipeline_operator_basic() {
    auto result = run_code("-3 |> abs;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Int);
    assert_eq(v.getInt(), 3);
}

void test_fold_alias_registered() {
    auto result = run_code("typeof(fold);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::String);
    assert_eq(v.getString(), "native_function");
}

void test_hash_comments_line_and_block() {
    auto result = run_code("# line comment\n1 + 1;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Int);
    assert_eq(v.getInt(), 2);
    
    result = run_code("### block\ncomment ###\n2 + 3;");
    assert_ok(result);
    
    value = result.value();
    assert_true(value.has_value());
    
    auto& v2 = value.value();
    assert_eq(v2.getType(), Value::Type::Int);
    assert_eq(v2.getInt(), 5);
}

void test_decimal_precision_argument() {
    auto result = run_code("decimal(1/3, 4);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(0.3333, v.getFloat(), 1e-10);
}

void test_log_family_semantics() {
    auto result = run_code("log(100);");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(2.0, v.getFloat(), 1e-10);
    
    result = run_code("ln(e());");
    assert_ok(result);
    
    value = result.value();
    assert_true(value.has_value());
    
    auto& v2 = value.value();
    assert_eq(v2.getType(), Value::Type::Float);
    assert_approx(1.0, v2.getFloat(), 1e-10);
    
    result = run_code("logBASE(2, 8);");
    assert_ok(result);
    
    value = result.value();
    assert_true(value.has_value());
    
    auto& v3 = value.value();
    assert_eq(v3.getType(), Value::Type::Float);
    assert_approx(3.0, v3.getFloat(), 1e-10);
}

void test_lsr002_constants_available() {
    auto result = run_code("EARTH_GRAVITY;");
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    
    auto& v = value.value();
    assert_eq(v.getType(), Value::Type::Float);
    assert_approx(9.80665, v.getFloat(), 1e-12);
    
    result = run_code("AVOGADRO;");
    assert_ok(result);
    
    value = result.value();
    assert_true(value.has_value());
    
    auto& v2 = value.value();
    assert_eq(v2.getType(), Value::Type::Float);
    assert_approx(6.02214076e23, v2.getFloat(), 1e-12);
}

int main() {
    TestRunner runner;
    
    runner.add_test("let_is_immutable", test_let_is_immutable);
    runner.add_test("let_member_assign_is_immutable", test_let_member_assign_is_immutable);
    runner.add_test("pipeline_operator_basic", test_pipeline_operator_basic);
    runner.add_test("fold_alias_registered", test_fold_alias_registered);
    runner.add_test("hash_comments_line_and_block", test_hash_comments_line_and_block);
    runner.add_test("decimal_precision_argument", test_decimal_precision_argument);
    runner.add_test("log_family_semantics", test_log_family_semantics);
    runner.add_test("lsr002_constants_available", test_lsr002_constants_available);
    
    return runner.run_all();
}
