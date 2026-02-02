use rumina::run_rumina;

#[test]
fn test_module_member_assignment() {
    // Test that we can assign to module members
    let result = run_rumina(
        r#"
        random.test_value = 42;
        random.test_value;
        "#,
    );
    assert!(result.is_ok(), "Module member assignment should work");
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "42");
    }
}

#[test]
fn test_module_member_read_after_write() {
    // Test that we can read back what we wrote
    let result = run_rumina(
        r#"
        time.custom = 100;
        time.custom + 50;
        "#,
    );
    assert!(result.is_ok(), "Should be able to read module member after write");
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "150");
    }
}

#[test]
fn test_module_member_overwrite() {
    // Test that we can overwrite module members
    let result = run_rumina(
        r#"
        random.value = 10;
        random.value = 20;
        random.value;
        "#,
    );
    assert!(result.is_ok(), "Should be able to overwrite module member");
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "20");
    }
}

#[test]
fn test_module_multiple_members() {
    // Test that we can have multiple custom members
    let result = run_rumina(
        r#"
        time.x = 1;
        time.y = 2;
        time.x + time.y;
        "#,
    );
    assert!(result.is_ok(), "Should be able to set multiple module members");
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "3");
    }
}

#[test]
fn test_module_access_builtin_member() {
    // Test that we can still access built-in module members
    let result = run_rumina(
        r#"
        typeof(random.rand);
        "#,
    );
    assert!(result.is_ok(), "Should be able to access built-in module members");
    if let Ok(Some(value)) = result {
        assert_eq!(value.to_string(), "native_function");
    }
}
