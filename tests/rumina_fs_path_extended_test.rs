use rumina::{Value, run_rumina_with_dir};
use std::fs;

#[test]
fn test_rumina_fs_realpath_link_symlink() {
    let temp_dir = std::env::temp_dir().join("rumina_fs_path_ext_test");
    fs::create_dir_all(&temp_dir).expect("create temp dir failed");

    let src = temp_dir.join("src.txt");
    let hard = temp_dir.join("hard.txt");
    let sym = temp_dir.join("sym.txt");

    let src_s = src.to_string_lossy().replace('\\', "\\\\");
    let hard_s = hard.to_string_lossy().replace('\\', "\\\\");
    let sym_s = sym.to_string_lossy().replace('\\', "\\\\");

    let code = format!(
        r#"
include "rumina:fs";
include "rumina:path";

fs.writeText("{src}", "hello");
fs.link("{src}", "{hard}");
fs.symlink("{src}", "{sym}");

var rp = fs.realpath("{src}");
var lk = fs.readLink("{sym}");
var ents = fs.readDir("{dir}", true);
var first_ok = size(ents) > 0 && ents[0].name != "";

var ok =
  fs.exists("{hard}") &&
  fs.exists("{sym}") &&
  path.basename(rp) == "src.txt" &&
  path.basename(lk) == "src.txt" &&
  first_ok;

if (ok) {{ "ok"; }} else {{ "bad"; }}
"#,
        src = src_s,
        hard = hard_s,
        sym = sym_s,
        dir = temp_dir.to_string_lossy().replace('\\', "\\\\")
    );

    let result = run_rumina_with_dir(&code, Some(temp_dir.to_string_lossy().to_string()))
        .expect("execution failed");

    match result {
        Some(Value::String(s)) => assert_eq!(s, "ok"),
        other => panic!("Expected Some(String), got {:?}", other),
    }

    fs::remove_dir_all(&temp_dir).ok();
}
