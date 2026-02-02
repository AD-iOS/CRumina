use rumina::ast::BinOp;
use rumina::interpreter::Interpreter;
use rumina::value::Value;

#[test]
fn test_string_greater_than() {
    let mut interpreter = Interpreter::new();
    let a = Value::String("apple".to_string());
    let b = Value::String("banana".to_string());
    
    // "banana" > "apple" should be true
    let result = interpreter.eval_binary_op(&b, BinOp::Greater, &a);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    // "apple" > "banana" should be false
    let result = interpreter.eval_binary_op(&a, BinOp::Greater, &b);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(false));
}

#[test]
fn test_string_greater_equal() {
    let mut interpreter = Interpreter::new();
    let a = Value::String("apple".to_string());
    let b = Value::String("banana".to_string());
    let c = Value::String("apple".to_string());
    
    // "banana" >= "apple" should be true
    let result = interpreter.eval_binary_op(&b, BinOp::GreaterEq, &a);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    // "apple" >= "apple" should be true (equal)
    let result = interpreter.eval_binary_op(&a, BinOp::GreaterEq, &c);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    // "apple" >= "banana" should be false
    let result = interpreter.eval_binary_op(&a, BinOp::GreaterEq, &b);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(false));
}

#[test]
fn test_string_less_than() {
    let mut interpreter = Interpreter::new();
    let a = Value::String("apple".to_string());
    let b = Value::String("banana".to_string());
    
    // "apple" < "banana" should be true
    let result = interpreter.eval_binary_op(&a, BinOp::Less, &b);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    // "banana" < "apple" should be false
    let result = interpreter.eval_binary_op(&b, BinOp::Less, &a);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(false));
}

#[test]
fn test_string_less_equal() {
    let mut interpreter = Interpreter::new();
    let a = Value::String("apple".to_string());
    let b = Value::String("banana".to_string());
    let c = Value::String("apple".to_string());
    
    // "apple" <= "banana" should be true
    let result = interpreter.eval_binary_op(&a, BinOp::LessEq, &b);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    // "apple" <= "apple" should be true (equal)
    let result = interpreter.eval_binary_op(&a, BinOp::LessEq, &c);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    // "banana" <= "apple" should be false
    let result = interpreter.eval_binary_op(&b, BinOp::LessEq, &a);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(false));
}

#[test]
fn test_string_lexicographic_ordering() {
    let mut interpreter = Interpreter::new();
    
    // Test various lexicographic orderings
    let test_cases = vec![
        ("abc", "abd", true),  // abc < abd
        ("abc", "abcd", true), // abc < abcd (shorter comes first)
        ("xyz", "abc", false), // xyz > abc
        ("", "a", true),       // empty string < non-empty
        ("a", "", false),      // non-empty > empty string
    ];
    
    for (left, right, expected) in test_cases {
        let left_val = Value::String(left.to_string());
        let right_val = Value::String(right.to_string());
        
        let result = interpreter.eval_binary_op(&left_val, BinOp::Less, &right_val);
        assert!(result.is_ok(), "Comparison failed for {} < {}", left, right);
        assert_eq!(
            result.unwrap(),
            Value::Bool(expected),
            "Expected {} < {} to be {}",
            left,
            right,
            expected
        );
    }
}

#[test]
fn test_string_equal_and_not_equal() {
    let mut interpreter = Interpreter::new();
    let a = Value::String("apple".to_string());
    let b = Value::String("banana".to_string());
    let c = Value::String("apple".to_string());
    
    // Equal
    let result = interpreter.eval_binary_op(&a, BinOp::Equal, &c);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    // Not Equal
    let result = interpreter.eval_binary_op(&a, BinOp::NotEqual, &b);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(true));
    
    let result = interpreter.eval_binary_op(&a, BinOp::NotEqual, &c);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), Value::Bool(false));
}
