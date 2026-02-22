use rumina::ast::BinOp;
use rumina::interpreter::Interpreter;
use rumina::value::Value;
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

#[test]
fn test_struct_not_equal_null() {
    let mut interpreter = Interpreter::new();
    let mut map = HashMap::new();
    map.insert("x".to_string(), Value::Int(10));
    let struct_val = Value::Struct(Rc::new(RefCell::new(map)));
    let null_val = Value::Null;

    // struct != null should be true
    let result = interpreter.eval_binary_op(&struct_val, BinOp::NotEqual, &null_val);
    assert!(result.is_ok(), "struct != null should be supported");
    assert_eq!(result.unwrap(), Value::Bool(true));
}

#[test]
fn test_struct_equal_null() {
    let mut interpreter = Interpreter::new();
    let mut map = HashMap::new();
    map.insert("x".to_string(), Value::Int(10));
    let struct_val = Value::Struct(Rc::new(RefCell::new(map)));
    let null_val = Value::Null;

    // struct == null should be false
    let result = interpreter.eval_binary_op(&struct_val, BinOp::Equal, &null_val);
    assert!(result.is_ok(), "struct == null should be supported");
    assert_eq!(result.unwrap(), Value::Bool(false));
}

#[test]
fn test_struct_equal_same_reference() {
    let mut interpreter = Interpreter::new();
    let mut map = HashMap::new();
    map.insert("x".to_string(), Value::Int(10));
    let struct_val = Value::Struct(Rc::new(RefCell::new(map)));
    let struct_val2 = struct_val.clone();

    // Same reference should be equal
    let result = interpreter.eval_binary_op(&struct_val, BinOp::Equal, &struct_val2);
    assert!(
        result.is_ok(),
        "struct == struct (same ref) should be supported"
    );
    assert_eq!(result.unwrap(), Value::Bool(true));
}

#[test]
fn test_struct_equal_different_reference() {
    let mut interpreter = Interpreter::new();
    let mut map1 = HashMap::new();
    map1.insert("x".to_string(), Value::Int(10));
    let struct_val1 = Value::Struct(Rc::new(RefCell::new(map1)));

    let mut map2 = HashMap::new();
    map2.insert("x".to_string(), Value::Int(10));
    let struct_val2 = Value::Struct(Rc::new(RefCell::new(map2)));

    // Different references should not be equal (even with same content)
    let result = interpreter.eval_binary_op(&struct_val1, BinOp::Equal, &struct_val2);
    assert!(
        result.is_ok(),
        "struct == struct (diff ref) should be supported"
    );
    assert_eq!(result.unwrap(), Value::Bool(false));
}

#[test]
fn test_struct_not_equal_different_reference() {
    let mut interpreter = Interpreter::new();
    let mut map1 = HashMap::new();
    map1.insert("x".to_string(), Value::Int(10));
    let struct_val1 = Value::Struct(Rc::new(RefCell::new(map1)));

    let mut map2 = HashMap::new();
    map2.insert("x".to_string(), Value::Int(10));
    let struct_val2 = Value::Struct(Rc::new(RefCell::new(map2)));

    // Different references should be not equal
    let result = interpreter.eval_binary_op(&struct_val1, BinOp::NotEqual, &struct_val2);
    assert!(
        result.is_ok(),
        "struct != struct (diff ref) should be supported"
    );
    assert_eq!(result.unwrap(), Value::Bool(true));
}

#[test]
fn test_struct_not_equal_other_types() {
    let mut interpreter = Interpreter::new();
    let mut map = HashMap::new();
    map.insert("x".to_string(), Value::Int(10));
    let struct_val = Value::Struct(Rc::new(RefCell::new(map)));

    // Test with int
    let int_val = Value::Int(10);
    let result = interpreter.eval_binary_op(&struct_val, BinOp::NotEqual, &int_val);
    assert!(result.is_ok(), "struct != int should be supported");
    assert_eq!(result.unwrap(), Value::Bool(true));

    // Test with string
    let string_val = Value::String("test".to_string());
    let result = interpreter.eval_binary_op(&struct_val, BinOp::NotEqual, &string_val);
    assert!(result.is_ok(), "struct != string should be supported");
    assert_eq!(result.unwrap(), Value::Bool(true));

    // Test with bool
    let bool_val = Value::Bool(true);
    let result = interpreter.eval_binary_op(&struct_val, BinOp::NotEqual, &bool_val);
    assert!(result.is_ok(), "struct != bool should be supported");
    assert_eq!(result.unwrap(), Value::Bool(true));
}

#[test]
fn test_struct_equal_other_types() {
    let mut interpreter = Interpreter::new();
    let mut map = HashMap::new();
    map.insert("x".to_string(), Value::Int(10));
    let struct_val = Value::Struct(Rc::new(RefCell::new(map)));

    // Test with int
    let int_val = Value::Int(10);
    let result = interpreter.eval_binary_op(&struct_val, BinOp::Equal, &int_val);
    assert!(result.is_ok(), "struct == int should be supported");
    assert_eq!(result.unwrap(), Value::Bool(false));

    // Test with string
    let string_val = Value::String("test".to_string());
    let result = interpreter.eval_binary_op(&struct_val, BinOp::Equal, &string_val);
    assert!(result.is_ok(), "struct == string should be supported");
    assert_eq!(result.unwrap(), Value::Bool(false));
}

#[test]
fn test_null_not_equal_struct() {
    let mut interpreter = Interpreter::new();
    let mut map = HashMap::new();
    map.insert("x".to_string(), Value::Int(10));
    let struct_val = Value::Struct(Rc::new(RefCell::new(map)));
    let null_val = Value::Null;

    // null != struct should be true (reversed order)
    let result = interpreter.eval_binary_op(&null_val, BinOp::NotEqual, &struct_val);
    assert!(result.is_ok(), "null != struct should be supported");
    assert_eq!(result.unwrap(), Value::Bool(true));
}
