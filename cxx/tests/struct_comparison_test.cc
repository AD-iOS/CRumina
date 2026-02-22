#include <test_framework.h>
#include <interpreter.h>
#include <value.h>
#include <ast.h>
#include <memory>
#include <unordered_map>

using namespace rumina;
using namespace rumina::test;

std::shared_ptr<std::unordered_map<std::string, Value>> create_test_struct() {
    auto map = std::make_shared<std::unordered_map<std::string, Value>>();
    (*map)["x"] = Value(static_cast<int64_t>(10));
    return map;
}

void test_struct_not_equal_null() {
    Interpreter interpreter;
    auto struct_val = Value::makeStruct(create_test_struct());
    Value null_val;

    auto result = interpreter.eval_binary_op(struct_val, BinOp::NotEqual, null_val);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());
}

void test_struct_equal_null() {
    Interpreter interpreter;
    auto struct_val = Value::makeStruct(create_test_struct());
    Value null_val;

    auto result = interpreter.eval_binary_op(struct_val, BinOp::Equal, null_val);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

void test_struct_equal_same_reference() {
    Interpreter interpreter;
    auto map = create_test_struct();
    auto struct_val1 = Value::makeStruct(map);
    auto struct_val2 = Value::makeStruct(map);  // Same underlying map

    auto result = interpreter.eval_binary_op(struct_val1, BinOp::Equal, struct_val2);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());
}

void test_struct_equal_different_reference() {
    Interpreter interpreter;
    auto struct_val1 = Value::makeStruct(create_test_struct());
    auto struct_val2 = Value::makeStruct(create_test_struct());  // Different map

    auto result = interpreter.eval_binary_op(struct_val1, BinOp::Equal, struct_val2);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

void test_struct_not_equal_different_reference() {
    Interpreter interpreter;
    auto struct_val1 = Value::makeStruct(create_test_struct());
    auto struct_val2 = Value::makeStruct(create_test_struct());

    auto result = interpreter.eval_binary_op(struct_val1, BinOp::NotEqual, struct_val2);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());
}

void test_struct_not_equal_other_types() {
    Interpreter interpreter;
    auto struct_val = Value::makeStruct(create_test_struct());

    Value int_val(static_cast<int64_t>(10));
    auto result = interpreter.eval_binary_op(struct_val, BinOp::NotEqual, int_val);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    Value string_val("test");
    result = interpreter.eval_binary_op(struct_val, BinOp::NotEqual, string_val);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());

    Value bool_val(true);
    result = interpreter.eval_binary_op(struct_val, BinOp::NotEqual, bool_val);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());
}

void test_struct_equal_other_types() {
    Interpreter interpreter;
    auto struct_val = Value::makeStruct(create_test_struct());

    Value int_val(static_cast<int64_t>(10));
    auto result = interpreter.eval_binary_op(struct_val, BinOp::Equal, int_val);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());

    Value string_val("test");
    result = interpreter.eval_binary_op(struct_val, BinOp::Equal, string_val);
    assert_ok(result);
    value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_false(value.getBool());
}

void test_null_not_equal_struct() {
    Interpreter interpreter;
    Value null_val;
    auto struct_val = Value::makeStruct(create_test_struct());

    auto result = interpreter.eval_binary_op(null_val, BinOp::NotEqual, struct_val);
    assert_ok(result);
    auto value = result.value();
    assert_eq(value.getType(), Value::Type::Bool);
    assert_true(value.getBool());
}

int main() {
    TestRunner runner;
    
    runner.add_test("struct_not_equal_null", test_struct_not_equal_null);
    runner.add_test("struct_equal_null", test_struct_equal_null);
    runner.add_test("struct_equal_same_reference", test_struct_equal_same_reference);
    runner.add_test("struct_equal_different_reference", test_struct_equal_different_reference);
    runner.add_test("struct_not_equal_different_reference", test_struct_not_equal_different_reference);
    runner.add_test("struct_not_equal_other_types", test_struct_not_equal_other_types);
    runner.add_test("struct_equal_other_types", test_struct_equal_other_types);
    runner.add_test("null_not_equal_struct", test_null_not_equal_struct);
    
    return runner.run_all();
}
