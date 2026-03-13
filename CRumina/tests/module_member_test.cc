#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>

using namespace rumina;
using namespace rumina::test;

void test_module_member_assignment() {
    auto result = run_code(
        "random.test_value = 42;"
        "random.test_value;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "42");
}

void test_module_member_read_after_write() {
    auto result = run_code(
        "time.custom = 100;"
        "time.custom + 50;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "150");
}

void test_module_member_overwrite() {
    auto result = run_code(
        "random.value = 10;"
        "random.value = 20;"
        "random.value;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "20");
}

void test_module_multiple_members() {
    auto result = run_code(
        "time.x = 1;"
        "time.y = 2;"
        "time.x + time.y;"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "3");
}

void test_module_access_builtin_member() {
    auto result = run_code(
        "typeof(random.rand);"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "native_function");
}

int main() {
    TestRunner runner;
    
    runner.add_test("module_member_assignment", test_module_member_assignment);
    runner.add_test("module_member_read_after_write", test_module_member_read_after_write);
    runner.add_test("module_member_overwrite", test_module_member_overwrite);
    runner.add_test("module_multiple_members", test_module_multiple_members);
    runner.add_test("module_access_builtin_member", test_module_access_builtin_member);
    
    return runner.run_all();
}
