use crate::builtin::buffer::{buffer_to_bytes, new_buffer_from_bytes};
use crate::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use std::rc::Rc;
use std::time::UNIX_EPOCH;

pub fn create_fs_module() -> Value {
    let mut ns = HashMap::new();

    insert_fn(&mut ns, "readText", fs_read_text);
    insert_fn(&mut ns, "readBytes", fs_read_bytes);
    insert_fn(&mut ns, "writeText", fs_write_text);
    insert_fn(&mut ns, "writeBytes", fs_write_bytes);
    insert_fn(&mut ns, "append", fs_append);

    insert_fn(&mut ns, "exists", fs_exists);
    insert_fn(&mut ns, "isFile", fs_is_file);
    insert_fn(&mut ns, "isDir", fs_is_dir);
    insert_fn(&mut ns, "stat", fs_stat);

    insert_fn(&mut ns, "makeDir", fs_make_dir);
    insert_fn(&mut ns, "makeDirAll", fs_make_dir_all);
    insert_fn(&mut ns, "readDir", fs_read_dir);
    insert_fn(&mut ns, "remove", fs_remove);
    insert_fn(&mut ns, "removeAll", fs_remove_all);
    insert_fn(&mut ns, "rename", fs_rename);
    insert_fn(&mut ns, "copy", fs_copy);

    Value::Module(Rc::new(RefCell::new(ns)))
}

fn insert_fn(
    ns: &mut HashMap<String, Value>,
    name: &str,
    func: fn(&[Value]) -> Result<Value, String>,
) {
    ns.insert(
        name.to_string(),
        Value::NativeFunction {
            name: format!("fs::{}", name),
            func,
        },
    );
}

fn as_path(arg: &Value, fn_name: &str) -> Result<String, String> {
    match arg {
        Value::String(s) => Ok(s.clone()),
        _ => Err(format!("{} expects path as string", fn_name)),
    }
}

fn expect_arity(args: &[Value], expected: usize, sig: &str) -> Result<(), String> {
    if args.len() != expected {
        return Err(format!("{} expects {} arguments", sig, expected - 1));
    }
    Ok(())
}

fn fs_read_text(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.readText(path)")?;
    let path = as_path(&args[1], "fs.readText")?;
    let content = fs::read_to_string(&path)
        .map_err(|e| format!("fs.readText failed for '{}': {}", path, e))?;
    Ok(Value::String(content))
}

fn fs_read_bytes(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.readBytes(path)")?;
    let path = as_path(&args[1], "fs.readBytes")?;
    let bytes =
        fs::read(&path).map_err(|e| format!("fs.readBytes failed for '{}': {}", path, e))?;
    Ok(new_buffer_from_bytes(bytes))
}

fn fs_write_text(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 3, "fs.writeText(path, text)")?;
    let path = as_path(&args[1], "fs.writeText")?;
    let text = match &args[2] {
        Value::String(s) => s,
        _ => return Err("fs.writeText expects text as string".to_string()),
    };

    fs::write(&path, text).map_err(|e| format!("fs.writeText failed for '{}': {}", path, e))?;
    Ok(Value::Null)
}

fn fs_write_bytes(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 3, "fs.writeBytes(path, data)")?;
    let path = as_path(&args[1], "fs.writeBytes")?;
    let bytes = match &args[2] {
        Value::Struct(_) => buffer_to_bytes(&args[2])?,
        _ => return Err("fs.writeBytes expects Buffer as data".to_string()),
    };

    fs::write(&path, bytes).map_err(|e| format!("fs.writeBytes failed for '{}': {}", path, e))?;
    Ok(Value::Null)
}

fn fs_append(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 3, "fs.append(path, data)")?;
    let path = as_path(&args[1], "fs.append")?;
    let mut current = if Path::new(&path).exists() {
        fs::read(&path).map_err(|e| format!("fs.append read failed for '{}': {}", path, e))?
    } else {
        Vec::new()
    };

    match &args[2] {
        Value::String(s) => current.extend_from_slice(s.as_bytes()),
        _ => current.extend_from_slice(&buffer_to_bytes(&args[2])?),
    }

    fs::write(&path, current)
        .map_err(|e| format!("fs.append write failed for '{}': {}", path, e))?;
    Ok(Value::Null)
}

fn fs_exists(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.exists(path)")?;
    let path = as_path(&args[1], "fs.exists")?;
    Ok(Value::Bool(Path::new(&path).exists()))
}

fn fs_is_file(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.isFile(path)")?;
    let path = as_path(&args[1], "fs.isFile")?;
    Ok(Value::Bool(Path::new(&path).is_file()))
}

fn fs_is_dir(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.isDir(path)")?;
    let path = as_path(&args[1], "fs.isDir")?;
    Ok(Value::Bool(Path::new(&path).is_dir()))
}

fn fs_stat(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.stat(path)")?;
    let path = as_path(&args[1], "fs.stat")?;
    let meta = fs::metadata(&path).map_err(|e| format!("fs.stat failed for '{}': {}", path, e))?;

    let modified_secs = match meta.modified() {
        Ok(m) => m
            .duration_since(UNIX_EPOCH)
            .map(|d| d.as_secs() as i64)
            .unwrap_or(0),
        Err(_) => 0,
    };

    let mut map = HashMap::new();
    map.insert("size".to_string(), Value::Int(meta.len() as i64));
    map.insert("isFile".to_string(), Value::Bool(meta.is_file()));
    map.insert("isDir".to_string(), Value::Bool(meta.is_dir()));
    map.insert("modifiedTime".to_string(), Value::Int(modified_secs));
    Ok(Value::Struct(Rc::new(RefCell::new(map))))
}

fn fs_make_dir(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.makeDir(path)")?;
    let path = as_path(&args[1], "fs.makeDir")?;
    fs::create_dir(&path).map_err(|e| format!("fs.makeDir failed for '{}': {}", path, e))?;
    Ok(Value::Null)
}

fn fs_make_dir_all(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.makeDirAll(path)")?;
    let path = as_path(&args[1], "fs.makeDirAll")?;
    fs::create_dir_all(&path).map_err(|e| format!("fs.makeDirAll failed for '{}': {}", path, e))?;
    Ok(Value::Null)
}

fn fs_read_dir(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.readDir(path)")?;
    let path = as_path(&args[1], "fs.readDir")?;

    let entries =
        fs::read_dir(&path).map_err(|e| format!("fs.readDir failed for '{}': {}", path, e))?;
    let mut list = Vec::new();
    for entry in entries {
        let entry = entry.map_err(|e| format!("fs.readDir entry error: {}", e))?;
        let name = entry.file_name().to_string_lossy().to_string();
        list.push(Value::String(name));
    }

    Ok(Value::Array(Rc::new(RefCell::new(list))))
}

fn fs_remove(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.remove(path)")?;
    let path = as_path(&args[1], "fs.remove")?;
    let p = Path::new(&path);

    if p.is_file() {
        fs::remove_file(&path)
            .map_err(|e| format!("fs.remove file failed for '{}': {}", path, e))?;
    } else if p.is_dir() {
        fs::remove_dir(&path).map_err(|e| format!("fs.remove dir failed for '{}': {}", path, e))?;
    } else {
        return Err(format!("fs.remove: path does not exist '{}'", path));
    }

    Ok(Value::Null)
}

fn fs_remove_all(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 2, "fs.removeAll(path)")?;
    let path = as_path(&args[1], "fs.removeAll")?;
    let p = Path::new(&path);

    if p.is_file() {
        fs::remove_file(&path)
            .map_err(|e| format!("fs.removeAll file failed for '{}': {}", path, e))?;
    } else if p.is_dir() {
        fs::remove_dir_all(&path)
            .map_err(|e| format!("fs.removeAll dir failed for '{}': {}", path, e))?;
    } else {
        return Err(format!("fs.removeAll: path does not exist '{}'", path));
    }

    Ok(Value::Null)
}

fn fs_rename(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 3, "fs.rename(oldPath, newPath)")?;
    let old_path = as_path(&args[1], "fs.rename")?;
    let new_path = as_path(&args[2], "fs.rename")?;
    fs::rename(&old_path, &new_path)
        .map_err(|e| format!("fs.rename failed '{}'=> '{}': {}", old_path, new_path, e))?;
    Ok(Value::Null)
}

fn fs_copy(args: &[Value]) -> Result<Value, String> {
    expect_arity(args, 3, "fs.copy(srcPath, destPath)")?;
    let src = as_path(&args[1], "fs.copy")?;
    let dst = as_path(&args[2], "fs.copy")?;
    fs::copy(&src, &dst).map_err(|e| format!("fs.copy failed '{}'=> '{}': {}", src, dst, e))?;
    Ok(Value::Null)
}
