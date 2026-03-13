#include <test_framework.h>
#include <interpreter.h>
#include <value.h>
#include <ast.h>

using namespace rumina;
using namespace rumina::test;

void test_string_greater_than() {
    Interpreter interpreter;
    Value a("apple");
    Value b("banana");

    auto result = interpreter.eval_binary_op(b, BinOp::Greater, a);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(a, BinOp::Greater, b);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

void test_string_greater_equal() {
    Interpreter interpreter;
    Value a("apple");
    Value b("banana");
    Value c("apple");

    auto result = interpreter.eval_binary_op(b, BinOp::GreaterEq, a);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(a, BinOp::GreaterEq, c);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(a, BinOp::GreaterEq, b);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

void test_string_less_than() {
    Interpreter interpreter;
    Value a("apple");
    Value b("banana");

    auto result = interpreter.eval_binary_op(a, BinOp::Less, b);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(b, BinOp::Less, a);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

void test_string_less_equal() {
    Interpreter interpreter;
    Value a("apple");
    Value b("banana");
    Value c("apple");

    auto result = interpreter.eval_binary_op(a, BinOp::LessEq, b);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(a, BinOp::LessEq, c);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(b, BinOp::LessEq, a);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

void test_string_lexicographic_ordering() {
    Interpreter interpreter;
    
    struct TestCase {
        const char* left;
        const char* right;
        bool expected;
    };
    
    TestCase cases[] = {
        {"abc", "abd", true},
        {"abc", "abcd", true},
        {"xyz", "abc", false},
        {"", "a", true},
        {"a", "", false}
    };
    
    for (const auto& tc : cases) {
        Value left(tc.left);
        Value right(tc.right);
        
        auto result = interpreter.eval_binary_op(left, BinOp::Less, right);
        assert_ok(result);
        auto value = result.value();
        assert_eq(value.getType(), Value::Type::Bool);
        assert_eq(value.getBool(), tc.expected);
    }
}

void test_string_equal_and_not_equal() {
    Interpreter interpreter;
    Value a("apple");
    Value b("banana");
    Value c("apple");

    auto result = interpreter.eval_binary_op(a, BinOp::Equal, c);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(a, BinOp::NotEqual, b);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    result = interpreter.eval_binary_op(a, BinOp::NotEqual, c);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

int main() {
    TestRunner runner;
    
    runner.add_test("string_greater_than", test_string_greater_than);
    runner.add_test("string_greater_equal", test_string_greater_equal);
    runner.add_test("string_less_than", test_string_less_than);
    runner.add_test("string_less_equal", test_string_less_equal);
    runner.add_test("string_lexicographic_ordering", test_string_lexicographic_ordering);
    runner.add_test("string_equal_and_not_equal", test_string_equal_and_not_equal);
    
    return runner.run_all();
}
