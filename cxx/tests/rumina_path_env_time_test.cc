#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>
#include <cstdlib>

using namespace rumina;
using namespace rumina::test;

void test_rumina_path_module() {
    auto result = run_code(
        "include \"rumina:path\";"
        "var j = path.join([\"/var\", \"log/\", \"app.txt\"]);"
        "var b = path.basename(j);"
        "var d = path.basename(path.dirname(j));"
        "var e = path.extname(j);"
        "var a = path.isAbsolute(j);"
        "var n = path.normalize(\"/a/b/../c/./file.txt\");"
        "if (b == \"app.txt\" && d == \"log\" && e == \".txt\" && a && path.basename(n) == \"file.txt\") {"
        "    \"ok\";"
        "} else {"
        "    \"bad\";"
        "}"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "ok");
}

void test_rumina_env_module() {
    std::string temp_dir = create_temp_dir("rumina_env_mod_test");
    
    std::string code = 
        "include \"rumina:env\";"
        "include \"rumina:process\";"
        "include \"rumina:path\";"
        "env.set(\"RUMINA_TEST_ENV\", \"abc\");"
        "var got = env.get(\"RUMINA_TEST_ENV\");"
        "var has1 = env.has(\"RUMINA_TEST_ENV\");"
        "var all = env.all();"
        "var listed = all.RUMINA_TEST_ENV;"
        "var keys = env.keys();"
        "var has_key = false;"
        "var i = 0;"
        "while (i < size(keys)) {"
        "    if (keys[i] == \"RUMINA_TEST_ENV\") {"
        "        has_key = true;"
        "    }"
        "    i = i + 1;"
        "}"
        "var args = process.args();"
        "var args_ok = size(args) >= 1;"
        "var old = process.cwd();"
        "process.setCwd(\"" + temp_dir + "\");"
        "var now = process.cwd();"
        "var cwd_ok = path.basename(now) == path.basename(\"" + temp_dir + "\");"
        "process.setCwd(old);"
        "var pid_ok = process.pid() > 0;"
        "env.remove(\"RUMINA_TEST_ENV\");"
        "var removed = env.get(\"RUMINA_TEST_ENV\");"
        "var has2 = env.has(\"RUMINA_TEST_ENV\");"
        "if (got == \"abc\" && has1 && listed == \"abc\" && has_key && args_ok && cwd_ok && pid_ok && removed == null && !has2) {"
        "    \"ok\";"
        "} else {"
        "    \"bad\";"
        "}";
    
    auto result = run_code_with_dir(code, temp_dir);
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "ok");
    
    remove_temp_dir(temp_dir);
}

void test_rumina_time_module_with_timer() {
    auto result = run_code(
        "include \"rumina:time\";"
        "var t0 = time.now();"
        "var h0 = time.hrtimeMs();"
        "var timer = time.startTimer();"
        "time.sleep(20);"
        "var t1 = time.now();"
        "var h1 = time.hrtimeMs();"
        "var em = timer.elapsedMs();"
        "var es = timer.elapsedSec();"
        "if (t1 >= t0 && h1 >= h0 && em >= 5 && es >= 0) {"
        "    \"ok\";"
        "} else {"
        "    \"bad\";"
        "}"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "ok");
}

void test_rumina_path_extended_apis() {
    auto result = run_code(
        "include \"rumina:path\";"
        "var p = path.parse(\"/tmp/demo.txt\");"
        "var f = path.format(p);"
        "var f2 = path.format({ name = \"demo\", ext = \"txt\" });"
        "var from = path.resolve([\".\", \"tests\"]);"
        "var to = path.resolve([\".\", \"tests\", \"x\", \"target.bin\"]);"
        "var rel = path.relative(from, to);"
        "var sep_ok = size(path.sep) == 1;"
        "var delim_ok = size(path.delimiter) == 1;"
        "if ("
        "  p.base == \"demo.txt\" &&"
        "  p.ext == \".txt\" &&"
        "  p.name == \"demo\" &&"
        "  path.basename(f) == \"demo.txt\" &&"
        "  path.basename(f2) == \"demo.txt\" &&"
        "  path.basename(rel) == \"target.bin\" &&"
        "  sep_ok &&"
        "  delim_ok"
        ") {"
        "  \"ok\";"
        "} else {"
        "  \"bad\";"
        "}"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "ok");
}

void test_rumina_process_extended_apis() {
    auto result = run_code(
        "include \"rumina:process\";"
        "include \"rumina:path\";"
        "var p = process.platform();"
        "var a = process.arch();"
        "var v = process.version();"
        "var e = process.execPath();"
        "if (size(p) > 0 && size(a) > 0 && size(v) > 1 && path.isAbsolute(e)) {"
        "  \"ok\";"
        "} else {"
        "  \"bad\";"
        "}"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "ok");
}

int main() {
    TestRunner runner;
    
    runner.add_test("rumina_path_module", test_rumina_path_module);
    runner.add_test("rumina_env_module", test_rumina_env_module);
    runner.add_test("rumina_time_module_with_timer", test_rumina_time_module_with_timer);
    runner.add_test("rumina_path_extended_apis", test_rumina_path_extended_apis);
    runner.add_test("rumina_process_extended_apis", test_rumina_process_extended_apis);
    
    return runner.run_all();
}
