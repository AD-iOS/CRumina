pub mod ast;
pub mod builtin;
pub mod compiler;
pub mod error;
pub mod interpreter;
pub mod lexer;
pub mod parser;
pub mod token;
pub mod value;
pub mod vm;
pub mod vm_ops;

// WASM 接口模块
#[cfg(target_arch = "wasm32")]
pub mod wasm;

pub use error::{ErrorType, RuminaError, StackFrame};
pub use interpreter::Interpreter;
pub use lexer::Lexer;
pub use parser::Parser;
pub use value::Value;
pub use compiler::Compiler;
pub use vm::VM;

/// Run Lamina code using the VM
pub fn run_vm(source: &str) -> Result<Option<Value>, RuminaError> {
    let mut lexer = Lexer::new(source.to_string());
    let tokens = lexer.tokenize();

    let mut parser = Parser::new(tokens);
    let ast = parser.parse().map_err(|e| RuminaError::runtime(e))?;

    let mut compiler = Compiler::new();
    let bytecode = compiler.compile(ast)?;

    let mut interpreter = Interpreter::new();
    let globals = interpreter.get_globals();
    let mut vm = VM::new(globals);
    vm.load(bytecode);
    
    vm.run()
}

/// Run Lamina code using the original AST interpreter (deprecated)
pub fn run_interpreter(source: &str) -> Result<Option<Value>, RuminaError> {
    let mut lexer = Lexer::new(source.to_string());
    let tokens = lexer.tokenize();

    let mut parser = Parser::new(tokens);
    let ast = parser.parse().map_err(|e| RuminaError::runtime(e))?;

    let mut interpreter = Interpreter::new();
    interpreter.interpret(ast)
}

/// Run Lamina code (uses VM by default for backward compatibility)
pub fn run(source: &str) -> Result<(), RuminaError> {
    // For now, keep using the AST interpreter for backward compatibility
    // Will switch to VM once it's fully implemented
    let mut lexer = Lexer::new(source.to_string());
    let tokens = lexer.tokenize();

    let mut parser = Parser::new(tokens);
    let ast = parser.parse().map_err(|e| RuminaError::runtime(e))?;

    let mut interpreter = Interpreter::new();
    interpreter.interpret(ast)?;

    Ok(())
}
