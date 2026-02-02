/// Tests for converting between fractions and decimals
use rumina::{run_rumina, Value};

#[test]
fn test_decimal_function_converts_rational_to_float() {
    // Test that decimal() converts rational to float
    let result = run_rumina("var x = 1/10; decimal(x);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        match value {
            Value::Float(f) => {
                assert!((f - 0.1).abs() < 1e-10, "decimal(1/10) should be 0.1, got {}", f);
            }
            _ => panic!("Expected Float, got {:?}", value),
        }
    }
}

#[test]
fn test_decimal_function_with_quarter() {
    // Test decimal(1/4) = 0.25
    let result = run_rumina("var x = 1/4; decimal(x);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        match value {
            Value::Float(f) => {
                assert!((f - 0.25).abs() < 1e-10, "decimal(1/4) should be 0.25, got {}", f);
            }
            _ => panic!("Expected Float, got {:?}", value),
        }
    }
}

#[test]
fn test_decimal_function_with_third() {
    // Test decimal(1/3) = 0.333...
    let result = run_rumina("var x = 1/3; decimal(x);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        match value {
            Value::Float(f) => {
                assert!((f - 0.333333333333).abs() < 1e-10, "decimal(1/3) should be ~0.333, got {}", f);
            }
            _ => panic!("Expected Float, got {:?}", value),
        }
    }
}

#[test]
fn test_decimal_literal_converted_to_rational() {
    // Test that 0.1 is stored as rational
    let result = run_rumina("var x = 0.1; typeof(x);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        match value {
            Value::String(s) => {
                assert_eq!(s, "rational", "0.1 should be stored as rational, got {}", s);
            }
            _ => panic!("Expected String, got {:?}", value),
        }
    }
}

#[test]
fn test_float_function_converts_rational() {
    // Test float() function converts rational to float
    // Note: using variable assignment to avoid keyword issues
    let result = run_rumina("var x = 3/4; var y = float(x); y;");
    assert!(result.is_ok(), "Failed to execute: {:?}", result);
    
    if let Ok(Some(value)) = result {
        match value {
            Value::Float(f) => {
                assert!((f - 0.75).abs() < 1e-10, "float(3/4) should be 0.75, got {}", f);
            }
            _ => panic!("Expected Float, got {:?}", value),
        }
    }
}

#[test]
fn test_rational_arithmetic_stays_exact() {
    // Test that rational arithmetic maintains precision
    let result = run_rumina("var x = 0.1; var y = 0.2; var z = x + y; z;");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        let display = format!("{}", value);
        assert_eq!(display, "3/10", "0.1 + 0.2 should display as 3/10, got {}", display);
    }
}

#[test]
fn test_decimal_maintains_precision_comparison() {
    // The key test: 0.1 + 0.2 == 0.3 should be true with rationals
    let result = run_rumina("0.1 + 0.2 == 0.3;");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        match value {
            Value::Bool(b) => {
                assert!(b, "0.1 + 0.2 should equal 0.3");
            }
            _ => panic!("Expected Bool, got {:?}", value),
        }
    }
}

#[test]
fn test_mixed_rational_and_decimal_conversion() {
    // Test converting result of decimal operation through decimal and back to float
    let result = run_rumina("var r = 0.1 + 0.2; var d = decimal(r); var f = float(d); f;");
    assert!(result.is_ok(), "Failed to execute: {:?}", result);
    
    if let Ok(Some(value)) = result {
        match value {
            Value::Float(f) => {
                assert!((f - 0.3).abs() < 1e-10, "Should get 0.3, got {}", f);
            }
            _ => panic!("Expected Float, got {:?}", value),
        }
    }
}

#[test]
fn test_typeof_after_conversion() {
    // Test that type changes after conversion
    let result = run_rumina(r#"
        var x = 1/2;
        var t1 = typeof(x);
        var y = decimal(x);
        var t2 = typeof(y);
        t1 + "," + t2;
    "#);
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        match value {
            Value::String(s) => {
                assert_eq!(s, "rational,float", "Types should be rational,float, got {}", s);
            }
            _ => panic!("Expected String, got {:?}", value),
        }
    }
}

#[test]
fn test_complex_rational_expression() {
    // Test complex expression with multiple decimals
    let result = run_rumina("(0.1 + 0.2) * (0.5 + 0.5);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        let display = format!("{}", value);
        assert_eq!(display, "3/10", "(0.1 + 0.2) * (0.5 + 0.5) should be 3/10, got {}", display);
    }
}
