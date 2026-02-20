use crate::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::path::{Component, Path, PathBuf};
use std::rc::Rc;

pub fn create_path_module() -> Value {
    let mut ns = HashMap::new();
    insert_fn(&mut ns, "join", path_join);
    insert_fn(&mut ns, "basename", path_basename);
    insert_fn(&mut ns, "dirname", path_dirname);
    insert_fn(&mut ns, "extname", path_extname);
    insert_fn(&mut ns, "isAbsolute", path_is_absolute);
    insert_fn(&mut ns, "normalize", path_normalize);
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
            name: format!("path::{}", name),
            func,
        },
    );
}

fn as_string(v: &Value, name: &str) -> Result<String, String> {
    match v {
        Value::String(s) => Ok(s.clone()),
        _ => Err(format!("{} expects string argument", name)),
    }
}

fn path_join(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.join expects 1 argument (paths)".to_string());
    }

    let parts = match &args[1] {
        Value::Array(arr) => arr,
        _ => return Err("path.join expects List<String>".to_string()),
    };

    let mut out = PathBuf::new();
    for part in parts.borrow().iter() {
        match part {
            Value::String(s) => out.push(s),
            _ => return Err("path.join expects List<String>".to_string()),
        }
    }

    Ok(Value::String(out.to_string_lossy().to_string()))
}

fn path_basename(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.basename expects 1 argument (path)".to_string());
    }

    let p = as_string(&args[1], "path.basename")?;
    let name = Path::new(&p)
        .file_name()
        .map(|s| s.to_string_lossy().to_string())
        .unwrap_or_default();
    Ok(Value::String(name))
}

fn path_dirname(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.dirname expects 1 argument (path)".to_string());
    }

    let p = as_string(&args[1], "path.dirname")?;
    let parent = Path::new(&p)
        .parent()
        .map(|s| s.to_string_lossy().to_string())
        .unwrap_or_default();
    Ok(Value::String(parent))
}

fn path_extname(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.extname expects 1 argument (path)".to_string());
    }

    let p = as_string(&args[1], "path.extname")?;
    let ext = Path::new(&p)
        .extension()
        .map(|s| format!(".{}", s.to_string_lossy()))
        .unwrap_or_default();
    Ok(Value::String(ext))
}

fn path_is_absolute(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.isAbsolute expects 1 argument (path)".to_string());
    }

    let p = as_string(&args[1], "path.isAbsolute")?;
    let looks_unix_absolute = p.starts_with('/') || p.starts_with('\\');
    Ok(Value::Bool(
        Path::new(&p).is_absolute() || looks_unix_absolute,
    ))
}

fn path_normalize(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.normalize expects 1 argument (path)".to_string());
    }

    let p = as_string(&args[1], "path.normalize")?;
    let mut out = PathBuf::new();

    for c in Path::new(&p).components() {
        match c {
            Component::CurDir => {}
            Component::ParentDir => {
                out.pop();
            }
            other => out.push(other.as_os_str()),
        }
    }

    Ok(Value::String(out.to_string_lossy().to_string()))
}
