use crate::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::env;
use std::process;
use std::rc::Rc;

pub fn create_process_module() -> Value {
    let mut ns = HashMap::new();
    insert_fn(&mut ns, "args", process_args);
    insert_fn(&mut ns, "cwd", process_cwd);
    insert_fn(&mut ns, "setCwd", process_set_cwd);
    insert_fn(&mut ns, "pid", process_pid);
    insert_fn(&mut ns, "exit", process_exit);
    insert_fn(&mut ns, "platform", process_platform);
    insert_fn(&mut ns, "arch", process_arch);
    insert_fn(&mut ns, "version", process_version);
    insert_fn(&mut ns, "execPath", process_exec_path);
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
            name: format!("process::{}", name),
            func,
        },
    );
}

fn as_string(v: &Value, fn_name: &str) -> Result<String, String> {
    match v {
        Value::String(s) => Ok(s.clone()),
        _ => Err(format!("{} expects string", fn_name)),
    }
}

fn process_args(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("process.args expects no arguments".to_string());
    }
    let values = env::args().map(Value::String).collect::<Vec<_>>();
    Ok(Value::Array(Rc::new(RefCell::new(values))))
}

fn process_cwd(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("process.cwd expects no arguments".to_string());
    }
    let cwd = env::current_dir().map_err(|e| format!("process.cwd failed: {}", e))?;
    Ok(Value::String(cwd.to_string_lossy().to_string()))
}

fn process_set_cwd(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("process.setCwd expects 1 argument (path)".to_string());
    }
    let p = as_string(&args[1], "process.setCwd")?;
    env::set_current_dir(&p).map_err(|e| format!("process.setCwd failed for '{}': {}", p, e))?;
    Ok(Value::Null)
}

fn process_pid(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("process.pid expects no arguments".to_string());
    }
    Ok(Value::Int(process::id() as i64))
}

fn process_exit(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("process.exit expects 1 argument (code)".to_string());
    }
    let code = args[1].to_int()?;
    process::exit(code as i32)
}

fn process_platform(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("process.platform expects no arguments".to_string());
    }
    Ok(Value::String(std::env::consts::OS.to_string()))
}

fn process_arch(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("process.arch expects no arguments".to_string());
    }
    Ok(Value::String(std::env::consts::ARCH.to_string()))
}

fn process_version(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("process.version expects no arguments".to_string());
    }
    Ok(Value::String(format!("v{}", env!("CARGO_PKG_VERSION"))))
}

fn process_exec_path(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("process.execPath expects no arguments".to_string());
    }
    let path = env::current_exe().map_err(|e| format!("process.execPath failed: {}", e))?;
    Ok(Value::String(path.to_string_lossy().to_string()))
}
