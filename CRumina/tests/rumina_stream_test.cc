#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>
#include <fstream>
#include <filesystem>

using namespace rumina;
using namespace rumina::test;

void test_rumina_stream_read_write_text_and_lines() {
    std::string temp_dir = create_temp_dir("rumina_stream_test_text");
    std::string file_path = temp_dir + "/lines.txt";
    
    std::string code = 
        "include \"rumina:stream\";"
        "include \"rumina:buffer\";"
        "var w = stream.openWrite(\"" + file_path + "\", false);"
        "w.isClosed();"
        "w.writeText(\"line1\\nline2\\n\");"
        "w.flush();"
        "w.close();"
        "var w_closed = w.isClosed();"
        "var r = stream.openRead(\"" + file_path + "\");"
        "var l1 = r.readUntil(\"\\n\");"
        "var l2 = r.readUntil(\"\\n\");"
        "var l3 = r.readUntil(\"\\n\");"
        "r.close();"
        "var r_closed = r.isClosed();"
        "if (l1.toText() == \"line1\" && l2.toText() == \"line2\" && l3 == null && w_closed && r_closed) {"
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

void test_rumina_stream_read_bytes_and_append_mode() {
    std::string temp_dir = create_temp_dir("rumina_stream_test_bytes");
    std::string file_path = temp_dir + "/data.bin";
    
    std::string code = 
        "include \"rumina:stream\";"
        "include \"rumina:buffer\";"
        "var b = Buffer.alloc(3);"
        "b.set(0, 65);"
        "b.set(1, 66);"
        "b.set(2, 67);"
        "var w1 = stream.openWrite(\"" + file_path + "\", false);"
        "w1.writeBytes(b);"
        "w1.close();"
        "var w2 = stream.openWrite(\"" + file_path + "\", true);"
        "w2.writeText(\"D\");"
        "w2.close();"
        "var r = stream.openRead(\"" + file_path + "\");"
        "var c1 = r.readBytes(2);"
        "var c2 = r.readBytes(2);"
        "var c3 = r.readBytes(2);"
        "var all_rest = r.readAll();"
        "r.close();"
        "if ("
        "  c1.toText() == \"AB\" &&"
        "  c2.toText() == \"CD\" &&"
        "  c3 == null &&"
        "  all_rest.length() == 0"
        ") {"
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

void test_rumina_stream_read_until_buffer_delimiter_and_max_bytes() {
    std::string temp_dir = create_temp_dir("rumina_stream_test_until");
    std::string file_path = temp_dir + "/chunks.bin";
    
    std::string code = 
        "include \"rumina:stream\";"
        "include \"rumina:buffer\";"
        "var w = stream.openWrite(\"" + file_path + "\", false);"
        "w.writeText(\"a::bb::ccc\");"
        "w.close();"
        "var delim = Buffer.alloc(2);"
        "delim.set(0, 58);"
        "delim.set(1, 58);"
        "var r = stream.openRead(\"" + file_path + "\");"
        "var p1 = r.readUntil(delim);"
        "var p2 = r.readUntil(delim, 2);"
        "var p3 = r.readUntil(delim);"
        "var p4 = r.readAll();"
        "r.close();"
        "if ("
        "  p1.toText() == \"a\" &&"
        "  p2.toText() == \"bb\" &&"
        "  p3.length() == 0 &&"
        "  p4.toText() == \"ccc\""
        ") {"
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

void test_rumina_stream_seek_and_tell() {
    std::string temp_dir = create_temp_dir("rumina_stream_test_seek_tell");
    std::string file_path = temp_dir + "/seek.txt";
    
    std::string code = 
        "include \"rumina:stream\";"
        "include \"rumina:buffer\";"
        "var w = stream.openWrite(\"" + file_path + "\", false);"
        "w.writeText(\"0123456789\");"
        "var wt1 = w.tell();"
        "w.seek(5);"
        "var wt2 = w.tell();"
        "w.writeText(\"X\");"
        "w.close();"
        "var r = stream.openRead(\"" + file_path + "\");"
        "var rt1 = r.tell();"
        "r.seek(4);"
        "var rt2 = r.tell();"
        "var part = r.readBytes(3);"
        "var rt3 = r.tell();"
        "r.close();"
        "if (wt1 == 10 && wt2 == 5 && rt1 == 0 && rt2 == 4 && part.toText() == \"4X6\" && rt3 == 7) {"
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

int main() {
    TestRunner runner;
    
    runner.add_test("rumina_stream_read_write_text_and_lines", 
                    test_rumina_stream_read_write_text_and_lines);
    runner.add_test("rumina_stream_read_bytes_and_append_mode", 
                    test_rumina_stream_read_bytes_and_append_mode);
    runner.add_test("rumina_stream_read_until_buffer_delimiter_and_max_bytes", 
                    test_rumina_stream_read_until_buffer_delimiter_and_max_bytes);
    runner.add_test("rumina_stream_seek_and_tell", 
                    test_rumina_stream_seek_and_tell);
    
    return runner.run_all();
}
