use rumina::{Value, run_rumina_with_dir};
use std::fs;

#[test]
fn test_rumina_path_module() {
    let code = r#"
include "rumina:path";

var j = path.join(["/var", "log/", "app.txt"]);
var b = path.basename(j);
var d = path.basename(path.dirname(j));
var e = path.extname(j);
var a = path.isAbsolute(j);
var n = path.normalize("/a/b/../c/./file.txt");

if (b == "app.txt" && d == "log" && e == ".txt" && a && path.basename(n) == "file.txt") {
    "ok";
} else {
    "bad";
}
"#;

    let result = run_rumina_with_dir(code, None).expect("execution failed");
    match result {
        Some(Value::String(s)) => assert_eq!(s, "ok"),
        other => panic!("Expected Some(String), got {:?}", other),
    }
}

#[test]
fn test_rumina_env_module() {
    let temp_dir = std::env::temp_dir().join("rumina_env_mod_test");
    fs::create_dir_all(&temp_dir).expect("create temp dir failed");
    let temp_dir_str = temp_dir.to_string_lossy().replace('\\', "\\\\");

    let code = format!(
        r#"
include "rumina:env";
include "rumina:process";
include "rumina:path";

env.set("RUMINA_TEST_ENV", "abc");
var got = env.get("RUMINA_TEST_ENV");
var has1 = env.has("RUMINA_TEST_ENV");
var all = env.all();
var listed = all.RUMINA_TEST_ENV;

var args = process.args();
var args_ok = size(args) >= 1;

var old = process.cwd();
process.setCwd("{temp_dir}");
var now = process.cwd();
var cwd_ok = path.basename(now) == path.basename("{temp_dir}");
process.setCwd(old);

var pid_ok = process.pid() > 0;

env.remove("RUMINA_TEST_ENV");
var removed = env.get("RUMINA_TEST_ENV");
var has2 = env.has("RUMINA_TEST_ENV");

if (got == "abc" && has1 && listed == "abc" && args_ok && cwd_ok && pid_ok && removed == null && !has2) {{
    "ok";
}} else {{
    "bad";
}}
"#,
        temp_dir = temp_dir_str
    );

    let result = run_rumina_with_dir(&code, None).expect("execution failed");
    match result {
        Some(Value::String(s)) => assert_eq!(s, "ok"),
        other => panic!("Expected Some(String), got {:?}", other),
    }

    fs::remove_dir_all(&temp_dir).ok();
}

#[test]
fn test_rumina_time_module_with_timer() {
    let code = r#"
include "rumina:time";

var t0 = time.now();
var timer = time.startTimer();
time.sleep(20);
var t1 = time.now();
var em = timer.elapsedMs();
var es = timer.elapsedSec();

if (t1 >= t0 && em >= 5 && es >= 0) {
    "ok";
} else {
    "bad";
}
"#;

    let result = run_rumina_with_dir(code, None).expect("execution failed");
    match result {
        Some(Value::String(s)) => assert_eq!(s, "ok"),
        other => panic!("Expected Some(String), got {:?}", other),
    }
}
