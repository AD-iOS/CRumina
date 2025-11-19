/// Tests for math built-in functions
use rumina::run_rumina;

#[test]
fn test_sqrt_with_irrational() {
    // Test sqrt with irrational value (7 + 2*sqrt(10))
    let result = run_rumina("sqrt(7 + 2*sqrt(10));");
    assert!(result.is_ok(), "sqrt should handle irrational values");
    
    if let Ok(Some(value)) = result {
        let value_str = value.to_string();
        let value_float: f64 = value_str.parse().expect("Result should be a float");
        
        // Expected value: sqrt(7 + 2*sqrt(10)) ≈ 3.6502815398728847
        let expected = 3.6502815398728847;
        let diff = (value_float - expected).abs();
        
        assert!(
            diff < 1e-10,
            "Result should be approximately {}, got {}",
            expected,
            value_float
        );
    }
}

#[test]
fn test_sqrt_basic() {
    // Test basic sqrt functionality
    let result = run_rumina("sqrt(4);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "2");
    }
}

#[test]
fn test_sqrt_non_perfect_square() {
    // Test sqrt of non-perfect square (should return irrational)
    let result = run_rumina("sqrt(2);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        let value_str = value.to_string();
        assert!(value_str.contains("√") || value_str.contains("1.414"));
    }
}

#[test]
fn test_nested_sqrt() {
    // Test nested sqrt: sqrt(sqrt(16))
    let result = run_rumina("sqrt(sqrt(16));");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "2");
    }
}

#[test]
fn test_sqrt_with_multiplication() {
    // Test sqrt(4 * 9) = 6
    let result = run_rumina("sqrt(4 * 9);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "6");
    }
}

#[test]
fn test_sqrt_negative() {
    // Test sqrt of negative number (should return complex)
    let result = run_rumina("sqrt(-4);");
    assert!(result.is_ok());
    
    if let Ok(Some(value)) = result {
        let value_str = value.to_string();
        // Should contain 'i' for imaginary unit
        assert!(value_str.contains("i") || value_str.contains("2i"));
    }
}
