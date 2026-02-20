use crate::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

const BUFFER_DATA_KEY: &str = "__bytes";

pub fn create_buffer_module() -> Value {
    let mut ns = HashMap::new();
    ns.insert(
        "alloc".to_string(),
        Value::NativeFunction {
            name: "Buffer::alloc".to_string(),
            func: buffer_alloc,
        },
    );
    Value::Module(Rc::new(RefCell::new(ns)))
}

pub fn new_buffer_from_bytes(bytes: Vec<u8>) -> Value {
    let mut fields = HashMap::new();
    let byte_values = bytes
        .into_iter()
        .map(|b| Value::Int(i64::from(b)))
        .collect::<Vec<_>>();

    fields.insert(
        BUFFER_DATA_KEY.to_string(),
        Value::Array(Rc::new(RefCell::new(byte_values))),
    );

    fields.insert(
        "length".to_string(),
        Value::NativeFunction {
            name: "Buffer::length".to_string(),
            func: buffer_length,
        },
    );
    fields.insert(
        "get".to_string(),
        Value::NativeFunction {
            name: "Buffer::get".to_string(),
            func: buffer_get,
        },
    );
    fields.insert(
        "set".to_string(),
        Value::NativeFunction {
            name: "Buffer::set".to_string(),
            func: buffer_set,
        },
    );
    fields.insert(
        "slice".to_string(),
        Value::NativeFunction {
            name: "Buffer::slice".to_string(),
            func: buffer_slice,
        },
    );
    fields.insert(
        "toText".to_string(),
        Value::NativeFunction {
            name: "Buffer::toText".to_string(),
            func: buffer_to_text,
        },
    );

    Value::Struct(Rc::new(RefCell::new(fields)))
}

pub fn buffer_to_bytes(value: &Value) -> Result<Vec<u8>, String> {
    let bytes_arr = get_buffer_array(value)?;
    let bytes = bytes_arr
        .borrow()
        .iter()
        .map(value_to_u8)
        .collect::<Result<Vec<_>, _>>()?;
    Ok(bytes)
}

fn get_buffer_array(value: &Value) -> Result<Rc<RefCell<Vec<Value>>>, String> {
    let s = match value {
        Value::Struct(s) => s,
        _ => return Err("Expected Buffer object".to_string()),
    };

    let guard = s.borrow();
    let bytes = guard
        .get(BUFFER_DATA_KEY)
        .ok_or_else(|| "Expected Buffer object".to_string())?;

    match bytes {
        Value::Array(arr) => Ok(Rc::clone(arr)),
        _ => Err("Invalid Buffer internal storage".to_string()),
    }
}

fn value_to_u8(v: &Value) -> Result<u8, String> {
    match v {
        Value::Int(n) if (0..=255).contains(n) => Ok(*n as u8),
        Value::Int(n) => Err(format!("Byte value out of range: {}", n)),
        _ => Err("Byte must be int".to_string()),
    }
}

fn get_index(args: &[Value], pos: usize) -> Result<usize, String> {
    let idx = args
        .get(pos)
        .ok_or_else(|| "Missing index argument".to_string())?
        .to_int()?;
    if idx < 0 {
        return Err("Index must be non-negative".to_string());
    }
    Ok(idx as usize)
}

fn buffer_alloc(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("Buffer.alloc expects 1 argument (size)".to_string());
    }

    let size = args[1].to_int()?;
    if size < 0 {
        return Err("Buffer size must be non-negative".to_string());
    }

    Ok(new_buffer_from_bytes(vec![0; size as usize]))
}

fn buffer_length(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("Buffer.length expects no arguments".to_string());
    }

    let arr = get_buffer_array(&args[0])?;
    Ok(Value::Int(arr.borrow().len() as i64))
}

fn buffer_get(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("Buffer.get expects 1 argument (index)".to_string());
    }

    let arr = get_buffer_array(&args[0])?;
    let idx = get_index(args, 1)?;
    let guard = arr.borrow();
    guard
        .get(idx)
        .cloned()
        .ok_or_else(|| format!("Buffer index out of bounds: {}", idx))
}

fn buffer_set(args: &[Value]) -> Result<Value, String> {
    if args.len() != 3 {
        return Err("Buffer.set expects 2 arguments (index, value)".to_string());
    }

    let arr = get_buffer_array(&args[0])?;
    let idx = get_index(args, 1)?;
    let value = value_to_u8(&args[2])?;

    let mut guard = arr.borrow_mut();
    if idx >= guard.len() {
        return Err(format!("Buffer index out of bounds: {}", idx));
    }
    guard[idx] = Value::Int(i64::from(value));

    Ok(Value::Null)
}

fn buffer_slice(args: &[Value]) -> Result<Value, String> {
    if args.len() != 3 {
        return Err("Buffer.slice expects 2 arguments (start, end)".to_string());
    }

    let arr = get_buffer_array(&args[0])?;
    let start = get_index(args, 1)?;
    let end = get_index(args, 2)?;

    let guard = arr.borrow();
    if start > end || end > guard.len() {
        return Err(format!(
            "Invalid slice range: start={}, end={}, length={}",
            start,
            end,
            guard.len()
        ));
    }

    let out = guard[start..end]
        .iter()
        .map(value_to_u8)
        .collect::<Result<Vec<_>, _>>()?;
    Ok(new_buffer_from_bytes(out))
}

fn buffer_to_text(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("Buffer.toText expects no arguments".to_string());
    }

    let bytes = buffer_to_bytes(&args[0])?;
    let text = String::from_utf8(bytes).map_err(|e| format!("Buffer is not valid UTF-8: {}", e))?;
    Ok(Value::String(text))
}
