use crate::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::{Mutex, OnceLock};
use std::thread;
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

static TIMER_SEQ: AtomicI64 = AtomicI64::new(1);
static TIMERS: OnceLock<Mutex<HashMap<i64, Instant>>> = OnceLock::new();
static BOOT_INSTANT: OnceLock<Instant> = OnceLock::new();

fn timers() -> &'static Mutex<HashMap<i64, Instant>> {
    TIMERS.get_or_init(|| Mutex::new(HashMap::new()))
}

pub fn create_time_module() -> Value {
    let mut ns = HashMap::new();
    insert_fn(&mut ns, "now", time_now);
    insert_fn(&mut ns, "hrtimeMs", time_hrtime_ms);
    insert_fn(&mut ns, "sleep", time_sleep);
    insert_fn(&mut ns, "startTimer", time_start_timer);
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
            name: format!("time::{}", name),
            func,
        },
    );
}

fn timer_method(name: &str, func: fn(&[Value]) -> Result<Value, String>) -> Value {
    Value::NativeFunction {
        name: format!("Timer::{}", name),
        func,
    }
}

fn timer_id_from_self(value: &Value) -> Result<i64, String> {
    let s = match value {
        Value::Struct(s) => s,
        _ => return Err("Timer method expects Timer object".to_string()),
    };

    let guard = s.borrow();
    match guard.get("__timer_id") {
        Some(Value::Int(id)) => Ok(*id),
        _ => Err("Invalid Timer object".to_string()),
    }
}

fn build_timer() -> Value {
    let id = TIMER_SEQ.fetch_add(1, Ordering::Relaxed);
    if let Ok(mut map) = timers().lock() {
        map.insert(id, Instant::now());
    }

    let mut fields = HashMap::new();
    fields.insert("__timer_id".to_string(), Value::Int(id));
    fields.insert(
        "elapsedMs".to_string(),
        timer_method("elapsedMs", timer_elapsed_ms),
    );
    fields.insert(
        "elapsedSec".to_string(),
        timer_method("elapsedSec", timer_elapsed_sec),
    );

    Value::Struct(Rc::new(RefCell::new(fields)))
}

fn elapsed(id: i64) -> Result<Duration, String> {
    let map = timers()
        .lock()
        .map_err(|_| "Timer store lock poisoned".to_string())?;
    let start = map
        .get(&id)
        .ok_or_else(|| "Timer has expired or is invalid".to_string())?;
    Ok(start.elapsed())
}

pub fn time_now(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("time.now expects no arguments".to_string());
    }

    let millis = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|e| format!("time.now failed: {}", e))?
        .as_millis() as i64;
    Ok(Value::Int(millis))
}

pub fn time_sleep(args: &[Value]) -> Result<Value, String> {
    if args.len() != 2 {
        return Err("time.sleep expects 1 argument (ms)".to_string());
    }

    let ms = args[1].to_int()?;
    if ms < 0 {
        return Err("time.sleep expects non-negative milliseconds".to_string());
    }
    thread::sleep(Duration::from_millis(ms as u64));
    Ok(Value::Null)
}

pub fn time_hrtime_ms(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("time.hrtimeMs expects no arguments".to_string());
    }
    let boot = BOOT_INSTANT.get_or_init(Instant::now);
    Ok(Value::Float(boot.elapsed().as_secs_f64() * 1000.0))
}

pub fn time_start_timer(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("time.startTimer expects no arguments".to_string());
    }
    Ok(build_timer())
}

fn timer_elapsed_ms(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("timer.elapsedMs expects no arguments".to_string());
    }
    let id = timer_id_from_self(&args[0])?;
    let ms = elapsed(id)?.as_secs_f64() * 1000.0;
    Ok(Value::Float(ms))
}

fn timer_elapsed_sec(args: &[Value]) -> Result<Value, String> {
    if args.len() != 1 {
        return Err("timer.elapsedSec expects no arguments".to_string());
    }
    let id = timer_id_from_self(&args[0])?;
    let sec = elapsed(id)?.as_secs_f64();
    Ok(Value::Float(sec))
}
