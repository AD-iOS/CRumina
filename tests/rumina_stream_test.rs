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
var l1 = r.readUntil("\n");
var l2 = r.readUntil("\n");
var l3 = r.readUntil("\n");
r.close();

if (l1.toText() == "line1" && l2.toText() == "line2" && l3 == null) {{
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

#[test]
fn test_rumina_stream_read_until_buffer_delimiter_and_max_bytes() {
    let temp_dir = std::env::temp_dir().join("rumina_stream_test_until");
    fs::create_dir_all(&temp_dir).expect("create temp dir failed");
    let file_path = temp_dir.join("chunks.bin");
    let file_path_str = file_path.to_string_lossy().replace('\\', "\\\\");

    let code = format!(
        r#"
include "rumina:stream";
include "rumina:buffer";

var w = stream.openWrite("{file}", false);
w.writeText("a::bb::ccc");
w.close();

var delim = Buffer.alloc(2);
delim.set(0, 58); // ':'
delim.set(1, 58); // ':'

var r = stream.openRead("{file}");
var p1 = r.readUntil(delim);
var p2 = r.readUntil(delim, 2);
var p3 = r.readUntil(delim);
var p4 = r.readAll();
r.close();

if (
  p1.toText() == "a" &&
  p2.toText() == "bb" &&
  p3.length() == 0 &&
  p4.toText() == "ccc"
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

#[test]
fn test_rumina_stream_seek_and_tell() {
    let temp_dir = std::env::temp_dir().join("rumina_stream_test_seek_tell");
    fs::create_dir_all(&temp_dir).expect("create temp dir failed");
    let file_path = temp_dir.join("seek.txt");
    let file_path_str = file_path.to_string_lossy().replace('\\', "\\\\");

    let code = format!(
        r#"
include "rumina:stream";
include "rumina:buffer";

var w = stream.openWrite("{file}", false);
w.writeText("0123456789");
var wt1 = w.tell();
w.seek(5);
var wt2 = w.tell();
w.writeText("X");
w.close();

var r = stream.openRead("{file}");
var rt1 = r.tell();
r.seek(4);
var rt2 = r.tell();
var part = r.readBytes(3);
var rt3 = r.tell();
r.close();

if (wt1 == 10 && wt2 == 5 && rt1 == 0 && rt2 == 4 && part.toText() == "4X6" && rt3 == 7) {{
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
