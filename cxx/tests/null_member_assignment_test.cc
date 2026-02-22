#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>

using namespace rumina;
using namespace rumina::test;

void test_null_member_assignment_basic() {
    auto result = run_code(
        "var x = null;"
        "x.name = \"test\";"
        "x.name;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "test");
}

void test_null_member_assignment_multiple() {
    auto result = run_code(
        "var y = null;"
        "y.age = 25;"
        "y.city = \"Tokyo\";"
        "y.age;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "25");
}

void test_null_member_assignment_access_after() {
    auto result = run_code(
        "var z = null;"
        "z.first = \"hello\";"
        "z.second = \"world\";"
        "z.first + \" \" + z.second;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "hello world");
}

void test_existing_struct_member_assignment() {
    auto result = run_code(
        "var s = {};"
        "s.field = \"existing struct\";"
        "s.field;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "existing struct");
}

void test_null_member_assignment_overwrites_null() {
    auto result = run_code(
        "var obj = null;"
        "obj.prop = 42;"
        "obj.prop;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "42");
}

int main() {
    TestRunner runner;
    
    runner.add_test("null_member_assignment_basic", test_null_member_assignment_basic);
    runner.add_test("null_member_assignment_multiple", test_null_member_assignment_multiple);
    runner.add_test("null_member_assignment_access_after", test_null_member_assignment_access_after);
    runner.add_test("existing_struct_member_assignment", test_existing_struct_member_assignment);
    runner.add_test("null_member_assignment_overwrites_null", test_null_member_assignment_overwrites_null);
    
    return runner.run_all();
}
