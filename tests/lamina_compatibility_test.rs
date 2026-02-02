/// Tests for lamina compatibility features
/// These tests verify fixes for issues that prevented lamina bootstrap files from running

#[test]
fn test_null_equality() {
    let result = rumina::run_rumina("null == null;").unwrap();
    match result {
        Some(rumina::Value::Bool(b)) => assert!(b),
        _ => panic!("Expected Bool(true)"),
    }
}

#[test]
fn test_null_inequality() {
    let result = rumina::run_rumina("null != null;").unwrap();
    match result {
        Some(rumina::Value::Bool(b)) => assert!(!b),
        _ => panic!("Expected Bool(false)"),
    }
}

#[test]
fn test_null_vs_nonnull_equality() {
    let result = rumina::run_rumina("null == 10;").unwrap();
    match result {
        Some(rumina::Value::Bool(b)) => assert!(!b),
        _ => panic!("Expected Bool(false)"),
    }
}

#[test]
fn test_null_vs_nonnull_inequality() {
    let result = rumina::run_rumina("null != 10;").unwrap();
    match result {
        Some(rumina::Value::Bool(b)) => assert!(b),
        _ => panic!("Expected Bool(true)"),
    }
}

#[test]
fn test_function_variable_call() {
    let code = r#"
        func test() {
            return 42;
        }
        var f = test;
        f();
    "#;
    let result = rumina::run_rumina(code).unwrap();
    match result {
        Some(rumina::Value::Int(n)) => assert_eq!(n, 42),
        _ => panic!("Expected Int(42)"),
    }
}

#[test]
fn test_function_in_struct_variable_call() {
    let code = r#"
        func add(a, b) {
            return a + b;
        }
        struct Ops {
            add_fn = add;
        };
        var my_add = Ops.add_fn;
        my_add(10, 20);
    "#;
    let result = rumina::run_rumina(code).unwrap();
    match result {
        Some(rumina::Value::Int(n)) => assert_eq!(n, 30),
        _ => panic!("Expected Int(30)"),
    }
}

#[test]
fn test_complex_null_conditions() {
    let code = r#"
        var a = null;
        var b = null;
        var c = 5;
        if (a == b) {
            if (a != c) {
                return "correct";
            }
        }
        return "wrong";
    "#;
    let result = rumina::run_rumina(code).unwrap();
    match result {
        Some(rumina::Value::String(s)) => assert_eq!(s, "correct"),
        _ => panic!("Expected String('correct')"),
    }
}
