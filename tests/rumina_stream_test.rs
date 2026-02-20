use rumina::{Value, run_rumina_with_dir};
use std::fs;

#[test]
fn test_rumina_stream_read_write_text_and_lines() {
    let temp_dir = std::env::temp_dir().join("rumina_stream_test_text");
    fs::create_dir_all(&temp_dir).expect("create temp dir failed");
    let file_path = temp_dir.join("lines.txt");
    let file_path_str = file_path.to_string_lossy().replace('\\', "\\\\");

    let code = format!(
        r#"
include "rumina:stream";
include "rumina:buffer";

var w = stream.openWrite("{file}", false);
w.writeText("line1\nline2\n");
w.flush();
w.close();

var r = stream.openRead("{file}");
var l1 = r.readLine();
var l2 = r.readLine();
var l3 = r.readLine();
r.close();

if (l1 == "line1" && l2 == "line2" && l3 == null) {{
    "ok";
}} else {{
    "bad";
}}
"#,
        file = file_path_str
    );

    let result = run_rumina_with_dir(&code, Some(temp_dir.to_string_lossy().to_string()))
        .expect("execution failed");
    match result {
        Some(Value::String(s)) => assert_eq!(s, "ok"),
        other => panic!("Expected Some(String), got {:?}", other),
    }

    fs::remove_dir_all(&temp_dir).ok();
}

#[test]
fn test_rumina_stream_read_bytes_and_append_mode() {
    let temp_dir = std::env::temp_dir().join("rumina_stream_test_bytes");
    fs::create_dir_all(&temp_dir).expect("create temp dir failed");
    let file_path = temp_dir.join("data.bin");
    let file_path_str = file_path.to_string_lossy().replace('\\', "\\\\");

    let code = format!(
        r#"
include "rumina:stream";
include "rumina:buffer";

var b = Buffer.alloc(3);
b.set(0, 65);
b.set(1, 66);
b.set(2, 67);

var w1 = stream.openWrite("{file}", false);
w1.writeBytes(b);
w1.close();

var w2 = stream.openWrite("{file}", true);
w2.writeText("D");
w2.close();

var r = stream.openRead("{file}");
var c1 = r.readBytes(2);
var c2 = r.readBytes(2);
var c3 = r.readBytes(2);
var all_rest = r.readAll();
r.close();

if (
  c1.toText() == "AB" &&
  c2.toText() == "CD" &&
  c3 == null &&
  all_rest.length() == 0
) {{
    "ok";
}} else {{
    "bad";
}}
"#,
        file = file_path_str
    );

    let result = run_rumina_with_dir(&code, Some(temp_dir.to_string_lossy().to_string()))
        .expect("execution failed");
    match result {
        Some(Value::String(s)) => assert_eq!(s, "ok"),
        other => panic!("Expected Some(String), got {:?}", other),
    }

    fs::remove_dir_all(&temp_dir).ok();
}
