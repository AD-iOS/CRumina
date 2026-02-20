use crate::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::env;
use std::rc::Rc;

pub fn create_env_module() -> Value {
    let mut ns = HashMap::new();
    insert_fn(&mut ns, "get", env_get);
    insert_fn(&mut ns, "set", env_set);
    insert_fn(&mut ns, "has", env_has);
    insert_fn(&mut ns, "remove", env_remove);
    insert_fn(&mut ns, "all", env_all);
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
            name: format!("env::{}", name),
            func,
        },
    );
}

fn get_str(v: &Value, fn_name: &str) -> Result<String, String> {
    match v {
        Value::String(s) => Ok(s.clone()),
        _ => Err(format!("{} expects string", fn_name)),
    }
}

fn env_get(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("env.get expects 1 argument (key)".to_string());
    }
    let key = get_str(&args[1], "env.get")?;
    match env::var(&key) {
        Ok(v) => Ok(Value::String(v)),
        Err(_) => Ok(Value::Null),
    }
}

fn env_set(args: &[Value]) -> Result<Value, String> {
    if args.len() != 3 {
        return Err("env.set expects 2 arguments (key, value)".to_string());
    }
    let key = get_str(&args[1], "env.set")?;
    let value = get_str(&args[2], "env.set")?;
    // SAFETY: process-wide env mutation is intended behavior for env.set.
    unsafe { env::set_var(key, value) };
    Ok(Value::Null)
}

fn env_has(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("env.has expects 1 argument (key)".to_string());
    }
    let key = get_str(&args[1], "env.has")?;
    Ok(Value::Bool(env::var(&key).is_ok()))
}

fn env_remove(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("env.remove expects 1 argument (key)".to_string());
    }
    let key = get_str(&args[1], "env.remove")?;
    // SAFETY: process-wide env mutation is intended behavior for env.remove.
    unsafe { env::remove_var(key) };
    Ok(Value::Null)
}

fn env_all(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("env.all expects no arguments".to_string());
    }
    let mut map = HashMap::new();
    for (k, v) in env::vars() {
        map.insert(k, Value::String(v));
    }
    Ok(Value::Struct(Rc::new(RefCell::new(map))))
}
