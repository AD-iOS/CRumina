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
    insert_fn(&mut ns, "resolve", path_resolve);
    insert_fn(&mut ns, "relative", path_relative);
    insert_fn(&mut ns, "parse", path_parse);
    insert_fn(&mut ns, "format", path_format);

    ns.insert(
        "sep".to_string(),
        Value::String(std::path::MAIN_SEPARATOR.to_string()),
    );
    ns.insert(
        "delimiter".to_string(),
        Value::String(if cfg!(windows) { ";" } else { ":" }.to_string()),
    );
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

fn path_resolve(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.resolve expects 1 argument (paths)".to_string());
    }

    let parts = match &args[1] {
        Value::Array(arr) => arr,
        _ => return Err("path.resolve expects List<String>".to_string()),
    };

    let mut out = std::env::current_dir().map_err(|e| format!("path.resolve failed: {}", e))?;
    for part in parts.borrow().iter() {
        match part {
            Value::String(s) => {
                let p = Path::new(s);
                if p.is_absolute() {
                    out = PathBuf::from(p);
                } else {
                    out.push(p);
                }
            }
            _ => return Err("path.resolve expects List<String>".to_string()),
        }
    }

    let normalized = normalize_path(&out);
    Ok(Value::String(normalized.to_string_lossy().to_string()))
}

fn normalize_path(path: &Path) -> PathBuf {
    let mut out = PathBuf::new();
    for c in path.components() {
        match c {
            Component::CurDir => {}
            Component::ParentDir => {
                out.pop();
            }
            other => out.push(other.as_os_str()),
        }
    }
    out
}

fn path_relative(args: &[Value]) -> Result<Value, String> {
    if args.len() != 3 {
        return Err("path.relative expects 2 arguments (from, to)".to_string());
    }

    let from = as_string(&args[1], "path.relative")?;
    let to = as_string(&args[2], "path.relative")?;
    let from_abs = normalize_path(&std::env::current_dir().unwrap_or_default().join(from));
    let to_abs = normalize_path(&std::env::current_dir().unwrap_or_default().join(to));

    match pathdiff::diff_paths(&to_abs, &from_abs) {
        Some(diff) => Ok(Value::String(diff.to_string_lossy().to_string())),
        None => Ok(Value::String(to_abs.to_string_lossy().to_string())),
    }
}

fn path_parse(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.parse expects 1 argument (path)".to_string());
    }
    let p = as_string(&args[1], "path.parse")?;
    let path = Path::new(&p);

    let root = path
        .components()
        .next()
        .and_then(|c| match c {
            Component::Prefix(prefix) => Some(prefix.as_os_str().to_string_lossy().to_string()),
            Component::RootDir => Some(std::path::MAIN_SEPARATOR.to_string()),
            _ => None,
        })
        .unwrap_or_default();

    let dir = path
        .parent()
        .map(|s| s.to_string_lossy().to_string())
        .unwrap_or_default();
    let base = path
        .file_name()
        .map(|s| s.to_string_lossy().to_string())
        .unwrap_or_default();
    let ext = path
        .extension()
        .map(|s| format!(".{}", s.to_string_lossy()))
        .unwrap_or_default();
    let name = path
        .file_stem()
        .map(|s| s.to_string_lossy().to_string())
        .unwrap_or_default();

    let mut map = HashMap::new();
    map.insert("root".to_string(), Value::String(root));
    map.insert("dir".to_string(), Value::String(dir));
    map.insert("base".to_string(), Value::String(base));
    map.insert("ext".to_string(), Value::String(ext));
    map.insert("name".to_string(), Value::String(name));
    Ok(Value::Struct(Rc::new(RefCell::new(map))))
}

fn get_obj_string(obj: &HashMap<String, Value>, key: &str) -> String {
    match obj.get(key) {
        Some(Value::String(s)) => s.clone(),
        _ => String::new(),
    }
}

fn path_format(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("path.format expects 1 argument (parts)".to_string());
    }

    let obj = match &args[1] {
        Value::Struct(s) => s.borrow(),
        _ => return Err("path.format expects object struct".to_string()),
    };

    let dir = get_obj_string(&obj, "dir");
    let root = get_obj_string(&obj, "root");
    let mut base = get_obj_string(&obj, "base");
    if base.is_empty() {
        let name = get_obj_string(&obj, "name");
        let mut ext = get_obj_string(&obj, "ext");
        if !ext.is_empty() && !ext.starts_with('.') {
            ext = format!(".{}", ext);
        }
        base = format!("{}{}", name, ext);
    }

    let out = if !dir.is_empty() {
        Path::new(&dir).join(base)
    } else if !root.is_empty() {
        Path::new(&root).join(base)
    } else {
        PathBuf::from(base)
    };
    Ok(Value::String(out.to_string_lossy().to_string()))
}
