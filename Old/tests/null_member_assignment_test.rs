use rumina::run_rumina;

#[test]
fn test_null_member_assignment_basic() {
    let code = r#"
        var x = null;
        x.name = "test";
        x.name;
    "#;

    let result = run_rumina(code);
    assert!(result.is_ok());
    let value = result.unwrap();
    assert!(value.is_some());
    assert_eq!(value.unwrap().to_string(), "test");
}

#[test]
fn test_null_member_assignment_multiple() {
    let code = r#"
        var y = null;
        y.age = 25;
        y.city = "Tokyo";
        y.age;
    "#;

    let result = run_rumina(code);
    assert!(result.is_ok());
    let value = result.unwrap();
    assert!(value.is_some());
    assert_eq!(value.unwrap().to_string(), "25");
}

#[test]
fn test_null_member_assignment_access_after() {
    let code = r#"
        var z = null;
        z.first = "hello";
        z.second = "world";
        z.first + " " + z.second;
    "#;

    let result = run_rumina(code);
    assert!(result.is_ok());
    let value = result.unwrap();
    assert!(value.is_some());
    assert_eq!(value.unwrap().to_string(), "hello world");
}

#[test]
fn test_existing_struct_member_assignment() {
    let code = r#"
        var s = {};
        s.field = "existing struct";
        s.field;
    "#;

    let result = run_rumina(code);
    assert!(result.is_ok());
    let value = result.unwrap();
    assert!(value.is_some());
    assert_eq!(value.unwrap().to_string(), "existing struct");
}

#[test]
fn test_null_member_assignment_overwrites_null() {
    let code = r#"
        var obj = null;
        obj.prop = 42;
        obj.prop;
    "#;

    let result = run_rumina(code);
    assert!(result.is_ok());
    let value = result.unwrap();
    assert!(value.is_some());
    assert_eq!(value.unwrap().to_string(), "42");
}
