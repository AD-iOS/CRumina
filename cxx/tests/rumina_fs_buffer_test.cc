#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>
#include <fstream>
#include <filesystem>

using namespace rumina;
using namespace rumina::test;

void test_rumina_buffer_basic_ops() {
    auto result = run_code(
        "include \"rumina:buffer\";"
        "var b = Buffer.alloc(5);"
        "b.set(0, 72);"
        "b.set(1, 101);"
        "b.set(2, 108);"
        "b.set(3, 108);"
        "b.set(4, 111);"
        "b.toText();"
    );
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "Hello");
}

void test_rumina_fs_text_and_bytes() {
    std::string temp_dir = create_temp_dir("rumina_fs_buffer_test");
    
    std::string text_path = temp_dir + "/a.txt";
    std::string bin_path = temp_dir + "/b.bin";
    
    std::string code = 
        "include \"rumina:buffer\";"
        "include \"rumina:fs\";"
        "fs.writeText(\"" + text_path + "\", \"Alpha\");"
        "fs.append(\"" + text_path + "\", \"Beta\");"
        "var ok1 = fs.exists(\"" + text_path + "\");"
        "var t = fs.readText(\"" + text_path + "\");"
        "var buf = Buffer.alloc(3);"
        "buf.set(0, 65);"
        "buf.set(1, 66);"
        "buf.set(2, 67);"
        "fs.writeBytes(\"" + bin_path + "\", buf);"
        "var r = fs.readBytes(\"" + bin_path + "\");"
        "var g0 = r.get(0);"
        "var g1 = r.get(1);"
        "var g2 = r.get(2);"
        "if (ok1 && t == \"AlphaBeta\" && g0 == 65 && g1 == 66 && g2 == 67) {"
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

void test_rumina_buffer_extended_apis() {
    auto result = run_code(
        "include \"rumina:buffer\";"
        "var a = Buffer.from(\"48656c6c6f\", \"hex\");"
        "var b = Buffer.from(\"SGk=\", \"base64\");"
        "var c = Buffer.from(\"abc\");"
        "var u = Buffer.from(\"SGk\", \"base64url\");"
        "var h2 = Buffer.from(\"48656c6c6fzz\", \"hex\");"
        "var d = Buffer.alloc(6);"
        "d.fill(120);"
        "var copied = c.copy(d, 1, 0, 3);"
        "var merged = Buffer.concat([a, b]);"
        "var idx = d.indexOf(\"abc\");"
        "var has = d.includes(\"bc\");"
        "var idx2 = a.indexOf(\"6c\", 0, \"hex\");"
        "var idx3 = a.indexOf(\"bG8=\", -2, \"base64\");"
        "var has2 = a.includes(\"bG8=\", 3, \"base64\");"
        "var eq = a.equals(Buffer.from(\"Hello\"));"
        "var cmp = a.compare(Buffer.from(\"Hellp\"));"
        "var sub = a.subarray(1, -1);"
        "if ("
        "  a.toText() == \"Hello\" &&"
        "  b.toText() == \"Hi\" &&"
        "  c.toHex() == \"616263\" &&"
        "  b.toBase64() == \"SGk=\" &&"
        "  b.toBase64Url() == \"SGk\" &&"
        "  u.toText() == \"Hi\" &&"
        "  h2.toText() == \"Hello\" &&"
        "  copied == 3 &&"
        "  merged.toText() == \"HelloHi\" &&"
        "  idx == 1 &&"
        "  has &&"
        "  idx2 == 2 &&"
        "  idx3 == 3 &&"
        "  has2 &&"
        "  eq &&"
        "  cmp == -1 &&"
        "  sub.toText() == \"ell\""
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

void test_rumina_fs_write_options_flag() {
    std::string temp_dir = create_temp_dir("rumina_fs_write_flag_test");
    
    std::string text_path = temp_dir + "/flag.txt";
    
    std::string code = 
        "include \"rumina:fs\";"
        "fs.writeText(\"" + text_path + "\", \"A\", { flag = \"w\" });"
        "fs.writeText(\"" + text_path + "\", \"B\", { flag = \"a\" });"
        "var t = fs.readText(\"" + text_path + "\");"
        "var h = fs.readText(\"" + text_path + "\", \"hex\");"
        "var b64 = fs.readText(\"" + text_path + "\", { encoding = \"base64\" });"
        "if (t == \"AB\" && h == \"4142\" && b64 == \"QUI=\") { \"ok\"; } else { \"bad\"; }";
    
    auto result = run_code_with_dir(code, temp_dir);
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "ok");
    
    remove_temp_dir(temp_dir);
}

int main() {
    TestRunner runner;
    
    runner.add_test("rumina_buffer_basic_ops", test_rumina_buffer_basic_ops);
    runner.add_test("rumina_fs_text_and_bytes", test_rumina_fs_text_and_bytes);
    runner.add_test("rumina_buffer_extended_apis", test_rumina_buffer_extended_apis);
    runner.add_test("rumina_fs_write_options_flag", test_rumina_fs_write_options_flag);
    
    return runner.run_all();
}
