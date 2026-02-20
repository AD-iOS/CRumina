use crate::builtin::buffer::{buffer_to_bytes, new_buffer_from_bytes};
use crate::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::fs::{File, OpenOptions};
use std::io::{BufRead, BufReader, BufWriter, Read, Write};
use std::rc::Rc;
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::{Mutex, OnceLock};

const READ_STREAM_ID_KEY: &str = "__read_stream_id";
const WRITE_STREAM_ID_KEY: &str = "__write_stream_id";

static READ_STREAM_SEQ: AtomicI64 = AtomicI64::new(1);
static WRITE_STREAM_SEQ: AtomicI64 = AtomicI64::new(1);
static READ_STREAMS: OnceLock<Mutex<HashMap<i64, BufReader<File>>>> = OnceLock::new();
static WRITE_STREAMS: OnceLock<Mutex<HashMap<i64, BufWriter<File>>>> = OnceLock::new();

fn readers() -> &'static Mutex<HashMap<i64, BufReader<File>>> {
    READ_STREAMS.get_or_init(|| Mutex::new(HashMap::new()))
}

fn writers() -> &'static Mutex<HashMap<i64, BufWriter<File>>> {
    WRITE_STREAMS.get_or_init(|| Mutex::new(HashMap::new()))
}

pub fn create_stream_module() -> Value {
    let mut ns = HashMap::new();
    insert_fn(&mut ns, "openRead", stream_open_read);
    insert_fn(&mut ns, "openWrite", stream_open_write);
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
            name: format!("stream::{}", name),
            func,
        },
    );
}

fn method(name: &str, func: fn(&[Value]) -> Result<Value, String>) -> Value {
    Value::NativeFunction {
        name: name.to_string(),
        func,
    }
}

fn as_string(v: &Value, fn_name: &str) -> Result<String, String> {
    match v {
        Value::String(s) => Ok(s.clone()),
        _ => Err(format!("{} expects string argument", fn_name)),
    }
}

fn as_bool(v: &Value, fn_name: &str) -> Result<bool, String> {
    match v {
        Value::Bool(b) => Ok(*b),
        _ => Err(format!("{} expects bool argument", fn_name)),
    }
}

fn read_stream_id_from_self(value: &Value) -> Result<i64, String> {
    let s = match value {
        Value::Struct(s) => s,
        _ => return Err("ReadStream method expects ReadStream object".to_string()),
    };
    let guard = s.borrow();
    match guard.get(READ_STREAM_ID_KEY) {
        Some(Value::Int(id)) => Ok(*id),
        _ => Err("Invalid ReadStream object".to_string()),
    }
}

fn write_stream_id_from_self(value: &Value) -> Result<i64, String> {
    let s = match value {
        Value::Struct(s) => s,
        _ => return Err("WriteStream method expects WriteStream object".to_string()),
    };
    let guard = s.borrow();
    match guard.get(WRITE_STREAM_ID_KEY) {
        Some(Value::Int(id)) => Ok(*id),
        _ => Err("Invalid WriteStream object".to_string()),
    }
}

fn build_read_stream(id: i64) -> Value {
    let mut fields = HashMap::new();
    fields.insert(READ_STREAM_ID_KEY.to_string(), Value::Int(id));
    fields.insert(
        "readBytes".to_string(),
        method("ReadStream::readBytes", read_stream_read_bytes),
    );
    fields.insert(
        "readLine".to_string(),
        method("ReadStream::readLine", read_stream_read_line),
    );
    fields.insert(
        "readAll".to_string(),
        method("ReadStream::readAll", read_stream_read_all),
    );
    fields.insert(
        "close".to_string(),
        method("ReadStream::close", read_stream_close),
    );
    Value::Struct(Rc::new(RefCell::new(fields)))
}

fn build_write_stream(id: i64) -> Value {
    let mut fields = HashMap::new();
    fields.insert(WRITE_STREAM_ID_KEY.to_string(), Value::Int(id));
    fields.insert(
        "writeBytes".to_string(),
        method("WriteStream::writeBytes", write_stream_write_bytes),
    );
    fields.insert(
        "writeText".to_string(),
        method("WriteStream::writeText", write_stream_write_text),
    );
    fields.insert(
        "flush".to_string(),
        method("WriteStream::flush", write_stream_flush),
    );
    fields.insert(
        "close".to_string(),
        method("WriteStream::close", write_stream_close),
    );
    Value::Struct(Rc::new(RefCell::new(fields)))
}

fn stream_open_read(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("stream.openRead expects 1 argument (path)".to_string());
    }
    let path = as_string(&args[1], "stream.openRead")?;
    let file =
        File::open(&path).map_err(|e| format!("stream.openRead failed for '{}': {}", path, e))?;

    let id = READ_STREAM_SEQ.fetch_add(1, Ordering::Relaxed);
    let mut map = readers()
        .lock()
        .map_err(|_| "ReadStream store lock poisoned".to_string())?;
    map.insert(id, BufReader::new(file));
    Ok(build_read_stream(id))
}

fn stream_open_write(args: &[Value]) -> Result<Value, String> {
    if args.len() != 3 {
        return Err("stream.openWrite expects 2 arguments (path, append)".to_string());
    }
    let path = as_string(&args[1], "stream.openWrite")?;
    let append = as_bool(&args[2], "stream.openWrite")?;

    let mut opts = OpenOptions::new();
    opts.create(true).write(true);
    if append {
        opts.append(true);
    } else {
        opts.truncate(true);
    }

    let file = opts
        .open(&path)
        .map_err(|e| format!("stream.openWrite failed for '{}': {}", path, e))?;

    let id = WRITE_STREAM_SEQ.fetch_add(1, Ordering::Relaxed);
    let mut map = writers()
        .lock()
        .map_err(|_| "WriteStream store lock poisoned".to_string())?;
    map.insert(id, BufWriter::new(file));
    Ok(build_write_stream(id))
}

fn read_stream_read_bytes(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("readStream.readBytes expects 1 argument (size)".to_string());
    }
    let id = read_stream_id_from_self(&args[0])?;
    let size = args[1].to_int()?;
    if size < 0 {
        return Err("readStream.readBytes expects non-negative size".to_string());
    }

    let mut map = readers()
        .lock()
        .map_err(|_| "ReadStream store lock poisoned".to_string())?;
    let reader = map
        .get_mut(&id)
        .ok_or_else(|| "ReadStream is closed or invalid".to_string())?;

    let mut buf = vec![0u8; size as usize];
    let n = reader
        .read(&mut buf)
        .map_err(|e| format!("readStream.readBytes failed: {}", e))?;
    if n == 0 {
        return Ok(Value::Null);
    }
    buf.truncate(n);
    Ok(new_buffer_from_bytes(buf))
}

fn read_stream_read_line(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("readStream.readLine expects no arguments".to_string());
    }
    let id = read_stream_id_from_self(&args[0])?;

    let mut map = readers()
        .lock()
        .map_err(|_| "ReadStream store lock poisoned".to_string())?;
    let reader = map
        .get_mut(&id)
        .ok_or_else(|| "ReadStream is closed or invalid".to_string())?;

    let mut bytes = Vec::new();
    let n = reader
        .read_until(b'\n', &mut bytes)
        .map_err(|e| format!("readStream.readLine failed: {}", e))?;
    if n == 0 {
        return Ok(Value::Null);
    }

    if bytes.ends_with(b"\n") {
        bytes.pop();
        if bytes.ends_with(b"\r") {
            bytes.pop();
        }
    }

    let line = String::from_utf8(bytes)
        .map_err(|e| format!("readStream.readLine invalid UTF-8: {}", e))?;
    Ok(Value::String(line))
}

fn read_stream_read_all(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("readStream.readAll expects no arguments".to_string());
    }
    let id = read_stream_id_from_self(&args[0])?;

    let mut map = readers()
        .lock()
        .map_err(|_| "ReadStream store lock poisoned".to_string())?;
    let reader = map
        .get_mut(&id)
        .ok_or_else(|| "ReadStream is closed or invalid".to_string())?;

    let mut out = Vec::new();
    reader
        .read_to_end(&mut out)
        .map_err(|e| format!("readStream.readAll failed: {}", e))?;
    Ok(new_buffer_from_bytes(out))
}

fn read_stream_close(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("readStream.close expects no arguments".to_string());
    }
    let id = read_stream_id_from_self(&args[0])?;
    let mut map = readers()
        .lock()
        .map_err(|_| "ReadStream store lock poisoned".to_string())?;
    map.remove(&id);
    Ok(Value::Null)
}

fn write_stream_write_bytes(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("writeStream.writeBytes expects 1 argument (data)".to_string());
    }
    let id = write_stream_id_from_self(&args[0])?;
    let bytes = match &args[1] {
        Value::Struct(_) => buffer_to_bytes(&args[1])?,
        _ => return Err("writeStream.writeBytes expects Buffer".to_string()),
    };

    let mut map = writers()
        .lock()
        .map_err(|_| "WriteStream store lock poisoned".to_string())?;
    let writer = map
        .get_mut(&id)
        .ok_or_else(|| "WriteStream is closed or invalid".to_string())?;
    writer
        .write_all(&bytes)
        .map_err(|e| format!("writeStream.writeBytes failed: {}", e))?;
    Ok(Value::Null)
}

fn write_stream_write_text(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("writeStream.writeText expects 1 argument (text)".to_string());
    }
    let id = write_stream_id_from_self(&args[0])?;
    let text = as_string(&args[1], "writeStream.writeText")?;

    let mut map = writers()
        .lock()
        .map_err(|_| "WriteStream store lock poisoned".to_string())?;
    let writer = map
        .get_mut(&id)
        .ok_or_else(|| "WriteStream is closed or invalid".to_string())?;
    writer
        .write_all(text.as_bytes())
        .map_err(|e| format!("writeStream.writeText failed: {}", e))?;
    Ok(Value::Null)
}

fn write_stream_flush(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("writeStream.flush expects no arguments".to_string());
    }
    let id = write_stream_id_from_self(&args[0])?;

    let mut map = writers()
        .lock()
        .map_err(|_| "WriteStream store lock poisoned".to_string())?;
    let writer = map
        .get_mut(&id)
        .ok_or_else(|| "WriteStream is closed or invalid".to_string())?;
    writer
        .flush()
        .map_err(|e| format!("writeStream.flush failed: {}", e))?;
    Ok(Value::Null)
}

fn write_stream_close(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("writeStream.close expects no arguments".to_string());
    }
    let id = write_stream_id_from_self(&args[0])?;

    let mut map = writers()
        .lock()
        .map_err(|_| "WriteStream store lock poisoned".to_string())?;
    if let Some(mut writer) = map.remove(&id) {
        writer
            .flush()
            .map_err(|e| format!("writeStream.close flush failed: {}", e))?;
    }
    Ok(Value::Null)
}
