#include <test_framework.h>
#include <run_rumina.h>
#include <value.h>
#include <fstream>
#include <filesystem>

using namespace rumina;
using namespace rumina::test;

void test_rumina_fs_realpath_link_symlink() {
    std::string temp_dir = create_temp_dir("rumina_fs_path_ext_test");
    
    std::string src = temp_dir + "/src.txt";
    std::string hard = temp_dir + "/hard.txt";
    std::string sym = temp_dir + "/sym.txt";
    
    std::ofstream src_file(src);
    src_file << "hello";
    src_file.close();
    
    std::string code = 
        "include \"rumina:fs\";"
        "include \"rumina:path\";"
        "fs.link(\"" + src + "\", \"" + hard + "\");"
        "fs.symlink(\"" + src + "\", \"" + sym + "\");"
        "var rp = fs.realpath(\"" + src + "\");"
        "var lk = fs.readLink(\"" + sym + "\");"
        "var ents = fs.readDir(\"" + temp_dir + "\", true);"
        "var first_ok = size(ents) > 0 && ents[0].name != \"\";"
        "var ok ="
        "  fs.exists(\"" + hard + "\") &&"
        "  fs.exists(\"" + sym + "\") &&"
        "  path.basename(rp) == \"src.txt\" &&"
        "  path.basename(lk) == \"src.txt\" &&"
        "  first_ok;"
        "if (ok) { \"ok\"; } else { \"bad\"; }";
    
    auto result = run_code_with_dir(code, temp_dir);
    assert_ok(result);
    
    auto value = result.value();
    assert_true(value.has_value());
    assert_eq(value.value().toString(), "ok");
    
    remove_temp_dir(temp_dir);
}

int main() {
    TestRunner runner;
    
    runner.add_test("rumina_fs_realpath_link_symlink", test_rumina_fs_realpath_link_symlink);
    
    return runner.run_all();
}
