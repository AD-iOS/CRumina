```ast.h
#pragma once

#include "fwd.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace rumina {

// LSR-005: Declared types for type annotations
enum class DeclaredType {
    Int,
    Float,
    Bool,
    String,
    Rational,
    Irrational,
    Complex,
    Array,
    BigInt
};

// 二元运算符
enum class BinOp {
    Add, // +
    Sub, // -
    Mul, // *
    Div, // /
    Mod, // %
    Pow, // ^

    Equal,     // ==
    NotEqual,  // !=
    Greater,   // >
    GreaterEq, // >=
    Less,      // <
    LessEq,    // <=

    And, // &&
    Or   // ||
};

// 一元运算符
enum class UnaryOp {
    Neg,       // -
    Not,       // !
    Factorial  // ! (后缀)
};

// 表达式基类
class Expr {
public:
    virtual ~Expr() = default;
    virtual std::string toString() const = 0;
};

// 字面量表达式
class IntExpr : public Expr {
public:
    int64_t value;
    explicit IntExpr(int64_t v) : value(v) {}
    std::string toString() const override;
};

class FloatExpr : public Expr {
public:
    double value;
    explicit FloatExpr(double v) : value(v) {}
    std::string toString() const override;
};

class StringExpr : public Expr {
public:
    std::string value;
    explicit StringExpr(std::string v) : value(std::move(v)) {}
    std::string toString() const override;
};

class BoolExpr : public Expr {
public:
    bool value;
    explicit BoolExpr(bool v) : value(v) {}
    std::string toString() const override;
};

class NullExpr : public Expr {
public:
    std::string toString() const override;
};

// 标识符
class IdentExpr : public Expr {
public:
    std::string name;
    explicit IdentExpr(std::string n) : name(std::move(n)) {}
    std::string toString() const override;
};

// 二元运算
class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    BinOp op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, BinOp o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}

    std::string toString() const override;
};

// 一元运算
class UnaryExpr : public Expr {
public:
    UnaryOp op;
    std::unique_ptr<Expr> expr;

    UnaryExpr(UnaryOp o, std::unique_ptr<Expr> e)
        : op(o), expr(std::move(e)) {}

    std::string toString() const override;
};

// 数组字面量
class ArrayExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    explicit ArrayExpr(std::vector<std::unique_ptr<Expr>> elems)
        : elements(std::move(elems)) {}

    std::string toString() const override;
};

// 结构体字面量
class StructExpr : public Expr {
public:
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields;

    explicit StructExpr(std::vector<std::pair<std::string, std::unique_ptr<Expr>>> f)
        : fields(std::move(f)) {}

    std::string toString() const override;
};

// 函数调用
class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> func;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::unique_ptr<Expr> f, std::vector<std::unique_ptr<Expr>> a)
        : func(std::move(f)), args(std::move(a)) {}

    std::string toString() const override;
};

// 成员访问
class MemberExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::string member;

    MemberExpr(std::unique_ptr<Expr> obj, std::string mem)
        : object(std::move(obj)), member(std::move(mem)) {}

    std::string toString() const override;
};

// 索引访问
class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<Expr> obj, std::unique_ptr<Expr> idx)
        : object(std::move(obj)), index(std::move(idx)) {}

    std::string toString() const override;
};

// Lambda表达式
class LambdaExpr : public Expr {
public:
    std::vector<std::string> params;
    std::unique_ptr<Stmt> body;
    bool is_simple;

    LambdaExpr(std::vector<std::string> p, std::unique_ptr<Stmt> b, bool simple)
        : params(std::move(p)), body(std::move(b)), is_simple(simple) {}

    std::string toString() const override;
};

// 命名空间访问
class NamespaceExpr : public Expr {
public:
    std::string module;
    std::string name;

    NamespaceExpr(std::string mod, std::string n)
        : module(std::move(mod)), name(std::move(n)) {}

    std::string toString() const override;
};

// 语句基类
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual std::string toString() const = 0;
};

// 变量声明
class VarDeclStmt : public Stmt {
public:
    std::string name;
    bool is_bigint;
    std::optional<DeclaredType> declared_type;
    std::unique_ptr<Expr> value;

    VarDeclStmt(std::string n, bool bigint, std::optional<DeclaredType> dt, std::unique_ptr<Expr> v)
        : name(std::move(n)), is_bigint(bigint), declared_type(dt), value(std::move(v)) {}

    std::string toString() const override;
};

// 不可变变量声明
class LetDeclStmt : public Stmt {
public:
    std::string name;
    bool is_bigint;
    std::optional<DeclaredType> declared_type;
    std::unique_ptr<Expr> value;

    LetDeclStmt(std::string n, bool bigint, std::optional<DeclaredType> dt, std::unique_ptr<Expr> v)
        : name(std::move(n)), is_bigint(bigint), declared_type(dt), value(std::move(v)) {}

    std::string toString() const override;
};

// 赋值
class AssignStmt : public Stmt {
public:
    std::string name;
    std::unique_ptr<Expr> value;

    AssignStmt(std::string n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}

    std::string toString() const override;
};

// 成员赋值
class MemberAssignStmt : public Stmt {
public:
    std::unique_ptr<Expr> object;
    std::string member;
    std::unique_ptr<Expr> value;

    MemberAssignStmt(std::unique_ptr<Expr> obj, std::string mem, std::unique_ptr<Expr> v)
        : object(std::move(obj)), member(std::move(mem)), value(std::move(v)) {}

    std::string toString() const override;
};

// 表达式语句
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    std::string toString() const override;
};

// 函数定义
class FuncDefStmt : public Stmt {
public:
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<Stmt>> body;
    std::vector<std::string> decorators;

    FuncDefStmt(std::string n, std::vector<std::string> p, 
                std::vector<std::unique_ptr<Stmt>> b, std::vector<std::string> d)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)), decorators(std::move(d)) {}

    std::string toString() const override;
};

// 返回语句
class ReturnStmt : public Stmt {
public:
    std::optional<std::unique_ptr<Expr>> expr;

    ReturnStmt() : expr(std::nullopt) {}
    explicit ReturnStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}

    std::string toString() const override;
};

// If语句
class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> then_branch;
    std::optional<std::vector<std::unique_ptr<Stmt>>> else_branch;

    IfStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> then_b,
           std::optional<std::vector<std::unique_ptr<Stmt>>> else_b)
        : condition(std::move(cond)), then_branch(std::move(then_b)), else_branch(std::move(else_b)) {}

    std::string toString() const override;
};

// While循环
class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Stmt>> b)
        : condition(std::move(cond)), body(std::move(b)) {}

    std::string toString() const override;
};

// For循环
class ForStmt : public Stmt {
public:
    std::optional<std::unique_ptr<Stmt>> init;
    std::optional<std::unique_ptr<Expr>> condition;
    std::optional<std::unique_ptr<Stmt>> update;
    std::vector<std::unique_ptr<Stmt>> body;

    ForStmt(std::optional<std::unique_ptr<Stmt>> i,
            std::optional<std::unique_ptr<Expr>> cond,
            std::optional<std::unique_ptr<Stmt>> upd,
            std::vector<std::unique_ptr<Stmt>> b)
        : init(std::move(i)), condition(std::move(cond)), update(std::move(upd)), body(std::move(b)) {}

    std::string toString() const override;
};

// Loop循环
class LoopStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> body;

    explicit LoopStmt(std::vector<std::unique_ptr<Stmt>> b) : body(std::move(b)) {}
    std::string toString() const override;
};

// Break
class BreakStmt : public Stmt {
public:
    std::string toString() const override;
};

// Continue
class ContinueStmt : public Stmt {
public:
    std::string toString() const override;
};

// Include模块
class IncludeStmt : public Stmt {
public:
    std::string path;

    explicit IncludeStmt(std::string p) : path(std::move(p)) {}
    std::string toString() const override;
};

// 块语句
class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
    std::string toString() const override;
};

// 空语句
class EmptyStmt : public Stmt {
public:
    std::string toString() const override;
};

// 辅助函数
std::string binOpToString(BinOp op);
std::string unaryOpToString(UnaryOp op);

} // namespace rumina
```

```bytecode_optimizer.h
#pragma once

#include "vm.h"
#include <vector>
#include <unordered_set>

namespace rumina {

// 字节码窥孔优化器
class BytecodeOptimizer {
public:
    BytecodeOptimizer() : modified_(false) {}

    // 优化字节码，返回true如果有任何优化被应用
    bool optimize(ByteCode& bytecode);

private:
    bool modified_;

    void eliminateDeadPushPop(ByteCode& bytecode);
    void eliminateRedundantDup(ByteCode& bytecode);
    void mergeConstantOperations(ByteCode& bytecode);
    void optimizeJumpChains(ByteCode& bytecode);
    void eliminateNoopPatterns(ByteCode& bytecode);
};

} // namespace rumina
```

```call.h
#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"
#include <vector>

namespace rumina {

// 函数调用实现（供解释器内部使用）
namespace call {

// 函数调用
Value call_function(Interpreter* interp, const Value& func, std::vector<Value> args);

// 方法调用（带self注入）
Value call_method(Interpreter* interp, const Value& func, const Value& self_obj, 
                  std::vector<Value> args);

// 高阶函数处理
Value handle_foreach(Interpreter* interp, const std::vector<Value>& args);
Value handle_map(Interpreter* interp, const std::vector<Value>& args);
Value handle_filter(Interpreter* interp, const std::vector<Value>& args);
Value handle_reduce(Interpreter* interp, const std::vector<Value>& args);

} // namespace call
} // namespace rumina
```

```cas.h
#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {

// 计算机代数系统函数
Value cas_parse(const std::vector<Value>& args);
Value cas_differentiate(const std::vector<Value>& args);
Value cas_solve_linear(const std::vector<Value>& args);
Value cas_evaluate_at(const std::vector<Value>& args);
Value cas_store(const std::vector<Value>& args);
Value cas_load(const std::vector<Value>& args);
Value cas_numerical_derivative(const std::vector<Value>& args);
Value cas_integrate(const std::vector<Value>& args);
Value cas_definite_integral(const std::vector<Value>& args);

// 别名（无cas_前缀）
inline Value parse(const std::vector<Value>& args) { return cas_parse(args); }
inline Value differentiate(const std::vector<Value>& args) { return cas_differentiate(args); }
inline Value solve_linear(const std::vector<Value>& args) { return cas_solve_linear(args); }
inline Value evaluate_at(const std::vector<Value>& args) { return cas_evaluate_at(args); }
inline Value store(const std::vector<Value>& args) { return cas_store(args); }
inline Value load(const std::vector<Value>& args) { return cas_load(args); }
inline Value numerical_derivative(const std::vector<Value>& args) { return cas_numerical_derivative(args); }
inline Value integrate(const std::vector<Value>& args) { return cas_integrate(args); }
inline Value definite_integral(const std::vector<Value>& args) { return cas_definite_integral(args); }

} // namespace builtin
} // namespace rumina
```

```compiler.h
#pragma once

#include "ast.h"
#include "vm.h"
#include "result.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace rumina {

// 符号表
struct SymbolInfo {
    std::string name;
    size_t depth;
};

class SymbolTable {
public:
    SymbolTable();
    
    void enterScope();
    void exitScope();
    void define(const std::string& name);
    const SymbolInfo* resolve(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
};

// 循环上下文
struct LoopContext {
    size_t continue_target;
    std::vector<size_t> break_patches;
};

// 字节码编译器
class Compiler {
public:
    Compiler();
    explicit Compiler(const std::string& current_dir);

    Result<ByteCode> compile(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    ByteCode bytecode_;
    SymbolTable symbols_;
    std::vector<LoopContext> loop_stack_;
    std::optional<size_t> current_line_;
    size_t lambda_counter_ = 0;
    std::unordered_set<std::string> included_files_;
    std::optional<std::string> current_dir_;
    std::unordered_map<std::string, std::string> module_namespaces_;

    void emit(OpCode op);
    size_t currentAddress() const;
    size_t emitJump(OpCode op);
    void patchJump(size_t address);

    void compileStmt(const Stmt* stmt);
    void compileExpr(const Expr* expr);
    void compileInclude(const std::string& path);
    
    std::string extractModuleName(const std::vector<std::unique_ptr<Stmt>>& statements, 
                                   const std::string& contents, 
                                   const std::string& path);
    void compileStmtWithNamespace(const Stmt* stmt, const std::string& namespace_);
};

} // namespace rumina
```

```convert.h
#pragma once

#include "value.h"
#include "ast.h"

namespace rumina {

// 类型转换函数（供解释器内部使用）
namespace convert {

Value convert_to_int(const Value& val);
Value convert_to_float(const Value& val);
Value convert_to_bool(const Value& val);
Value convert_to_string(const Value& val);
Value convert_to_rational(const Value& val);
Value convert_to_irrational(const Value& val);
Value convert_to_complex(const Value& val);
Value convert_to_array(const Value& val);
Value convert_to_bigint(const Value& val);

Value convert_to_declared_type(const Value& val, DeclaredType dtype);

} // namespace convert
} // namespace rumina
```

```error.h
#pragma once

#include "fwd.h"
#include <string>
#include <vector>
#include <optional>

namespace rumina {

// 错误类型 - 使用独立枚举，不与 ast.h 冲突
enum class RuminaErrorType {
    RuntimeError,
    TypeError,
    IndexError,
    KeyError,
    DivisionByZeroError,
    UndefinedVariableError
};

// 栈帧
struct StackFrame {
    std::string function_name;
    std::string file_name;
    std::optional<size_t> line_number;
};

// 运行时错误
class RuminaError : public std::exception {
public:
    RuminaError(RuminaErrorType type, const std::string& msg);
    
    static RuminaError runtime(const std::string& msg);
    static RuminaError typeError(const std::string& msg);
    static RuminaError indexError(const std::string& msg);
    static RuminaError keyError(const std::string& msg);
    static RuminaError divisionByZero();
    static RuminaError undefinedVariable(const std::string& name);

    void addFrame(const StackFrame& frame);
    
    RuminaErrorType getType() const { return type_; }
    const std::string& getMessage() const { return message_; }
    const std::vector<StackFrame>& getStackTrace() const { return stack_trace_; }
    
    std::string formatError() const;
    const char* what() const noexcept override;

private:
    RuminaErrorType type_;
    std::string message_;
    std::vector<StackFrame> stack_trace_;
    mutable std::string what_cache_;
};

} // namespace rumina
```

```expr.h
#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"

namespace rumina {

// 表达式求值实现（供解释器内部使用）
namespace expr {

Value eval_expr(Interpreter* interp, const Expr* expr);

} // namespace expr
} // namespace rumina
```

```fwd.h
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace rumina {

// 前向声明所有需要的类型
class Value;
class IrrationalValue;
class Stmt;
class Expr;
class IntExpr;
class FloatExpr;
class StringExpr;
class BoolExpr;
class NullExpr;
class IdentExpr;
class BinaryExpr;
class UnaryExpr;
class ArrayExpr;
class StructExpr;
class CallExpr;
class MemberExpr;
class IndexExpr;
class LambdaExpr;
class NamespaceExpr;
class VarDeclStmt;
class LetDeclStmt;
class AssignStmt;
class MemberAssignStmt;
class ExprStmt;
class FuncDefStmt;
class ReturnStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class LoopStmt;
class BreakStmt;
class ContinueStmt;
class IncludeStmt;
class BlockStmt;
class EmptyStmt;

// 枚举声明
enum class DeclaredType;
enum class BinOp;
enum class UnaryOp;

// 类型别名
using NativeFunction = std::function<Value(const std::vector<Value>&)>;

} // namespace rumina
```

```interpreter.h
#pragma once

#include "value.h"
#include "ast.h"
#include "error.h"
#include "result.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

namespace rumina {

// 解释器
class Interpreter {
public:
    Interpreter();
    
    void setFile(const std::string& filename);
    std::shared_ptr<std::unordered_map<std::string, Value>> getGlobals();
    
    Result<std::optional<Value>> interpret(
        std::vector<std::unique_ptr<Stmt>> statements);
    
    Result<Value> eval_expr(const Expr* expr);
    Result<Value> eval_binary_op(const Value& left, BinOp op, const Value& right);
    Result<Value> eval_unary_op(UnaryOp op, const Value& val);
    
    Result<Value> call_function(const Value& func, std::vector<Value> args);
    Result<Value> call_method(const Value& func, const Value& self_obj, 
                              std::vector<Value> args);

private:
    std::shared_ptr<std::unordered_map<std::string, Value>> globals_;
    std::unordered_set<std::string> immutable_globals_;
    std::vector<std::shared_ptr<std::unordered_map<std::string, Value>>> locals_;
    std::vector<std::unordered_set<std::string>> immutable_locals_;
    
    std::optional<Value> return_value_;
    bool break_flag_ = false;
    bool continue_flag_ = false;
    
    std::string current_file_;
    std::vector<std::string> call_stack_;
    
    size_t recursion_depth_ = 0;
    static constexpr size_t MAX_RECURSION_DEPTH = 4000;
    
    void setVariable(const std::string& name, const Value& value, bool immutable);
    bool isImmutableBinding(const std::string& name) const;
    void assignVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name) const;
    bool variableExists(const std::string& name) const;
    
    Value applyDecorator(const std::string& decorator, const Value& func);
    
    Result<Value> eval_expr_impl(const Expr* expr);
    void execute_stmt(const Stmt* stmt);
    
    Result<Value> handle_foreach(const std::vector<Value>& args);
    Result<Value> handle_map(const std::vector<Value>& args);
    Result<Value> handle_filter(const std::vector<Value>& args);
    Result<Value> handle_reduce(const std::vector<Value>& args);
    
    Result<Value> compute_power(double base, double exponent);
    Result<Value> multiply_irrationals(const IrrationalValue& a, const IrrationalValue& b);
    
    RuminaError wrapError(const std::string& message) const;
};

} // namespace rumina
```

```lexer.h
#pragma once

#include "token.h"
#include <string>
#include <vector>
#include <optional>

namespace rumina {

class Lexer {
public:
    explicit Lexer(std::string input);

    Token nextToken();
    std::vector<Token> tokenize();

    size_t getLine() const { return line_; }
    size_t getColumn() const { return column_; }

private:
    std::string input_;
    size_t position_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;
    std::optional<char> current_char_;

    void advance();
    std::optional<char> peek() const;
    std::optional<char> peekN(size_t n) const;
    void skipWhitespace();
    void skipComment();

    Token readNumber();
    Token readString();
    Token readSingleString();
    Token readIdentifier();
};

} // namespace rumina
```

```operators.h
#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"

namespace rumina {

// 运算符实现（供解释器内部使用）
namespace operators {

Value compute_power(Interpreter* interp, double base, double exponent);
Value multiply_irrationals(Interpreter* interp, const IrrationalValue& a, const IrrationalValue& b);

// 二元运算
Value eval_binary_op(Interpreter* interp, const Value& left, BinOp op, const Value& right);

// 一元运算
Value eval_unary_op(Interpreter* interp, UnaryOp op, const Value& val);

} // namespace operators
} // namespace rumina
```

```optimizer.h
#pragma once

#include "ast.h"
#include "error.h"
#include "result.h"
#include <memory>
#include <vector>
#include <optional>

namespace rumina {

// AST优化器
class ASTOptimizer {
public:
    ASTOptimizer() : modified_(false) {}

    Result<std::vector<std::unique_ptr<Stmt>>> optimize(
        std::vector<std::unique_ptr<Stmt>> statements);

private:
    bool modified_;

    std::optional<std::unique_ptr<Stmt>> optimizeStmt(
        std::unique_ptr<Stmt> stmt);

    std::unique_ptr<Expr> optimizeExpr(std::unique_ptr<Expr> expr);

    std::optional<std::vector<std::unique_ptr<Stmt>>> tryUnrollForLoop(
        const std::optional<std::unique_ptr<Stmt>>& init,
        const std::optional<std::unique_ptr<Expr>>& condition,
        const std::optional<std::unique_ptr<Stmt>>& update,
        const std::vector<std::unique_ptr<Stmt>>& body);

    std::unique_ptr<Stmt> substituteVarInStmt(
        const Stmt* stmt, const std::string& var, int64_t val);

    std::unique_ptr<Expr> substituteVarInExpr(
        const Expr* expr, const std::string& var, int64_t val);
};

} // namespace rumina
```

```parser.h
#pragma once

#include "ast.h"
#include "token.h"
#include <vector>
#include <memory>

namespace rumina {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::vector<std::unique_ptr<Stmt>> parse();

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;

    const Token& currentToken() const;
    Token advance();
    bool match(TokenType type);
    bool peek(TokenType type) const;
    bool check(TokenType type) const;
    Token expect(TokenType type, const std::string& error_msg);
    Token previous() const;

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseVarDeclWithType(std::optional<DeclaredType> declared_type, bool immutable);
    std::unique_ptr<Stmt> parseStructDecl();
    std::unique_ptr<Stmt> parseDecoratedFuncDef();
    std::unique_ptr<Stmt> parseFuncDefWithDecorators(std::vector<std::string> decorators);
    std::unique_ptr<Stmt> parseReturn();
    std::unique_ptr<Stmt> parseIf();
    std::unique_ptr<Stmt> parseWhile();
    std::unique_ptr<Stmt> parseFor();
    std::unique_ptr<Stmt> parseLoop();
    std::unique_ptr<Stmt> parseInclude();
    std::unique_ptr<Stmt> parseBlock();
    std::vector<std::unique_ptr<Stmt>> parseBlockStatements();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parsePipeline();
    std::unique_ptr<Expr> parseOr();
    std::unique_ptr<Expr> parseAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAddition();
    std::unique_ptr<Expr> parseMultiplication();
    std::unique_ptr<Expr> parsePower();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::vector<std::unique_ptr<Expr>> parseArguments();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseArray();
    std::unique_ptr<Expr> parseStruct();
    std::unique_ptr<Expr> parseLambda(bool is_simple);

    std::unique_ptr<Expr> decimalToRational(const std::string& decimal_str);
    std::unique_ptr<Expr> transformPipeline(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);
};

} // namespace rumina
```

```result.h
#pragma once

#include <string>
#include <optional>

namespace rumina {

// Result类型，类似于Rust的Result<T, E>
template<typename T>
class Result {
private:
    bool is_ok_;
    T value_;
    std::string error_;

public:
    Result(T value) : is_ok_(true), value_(std::move(value)) {}
    Result(const std::string& error) : is_ok_(false), error_(error) {}
    Result(const char* error) : is_ok_(false), error_(error) {}
    
    bool is_ok() const { return is_ok_; }
    bool is_error() const { return !is_ok_; }
    
    T value() const { return value_; }
    T& value() { return value_; }
    
    const std::string& error() const { return error_; }
    
    std::optional<T> ok() const {
        if (is_ok_) return value_;
        return std::nullopt;
    }
};

// 辅助函数
template<typename T>
Result<T> Ok(T value) {
    return Result<T>(std::move(value));
}

template<typename T>
Result<T> Err(const std::string& error) {
    return Result<T>(error);
}

template<typename T>
Result<T> Err(const char* error) {
    return Result<T>(std::string(error));
}

// 对于返回void的情况
class VoidResult {
private:
    bool is_ok_;
    std::string error_;

public:
    VoidResult() : is_ok_(true) {}
    VoidResult(const std::string& error) : is_ok_(false), error_(error) {}
    VoidResult(const char* error) : is_ok_(false), error_(error) {}
    
    bool is_ok() const { return is_ok_; }
    bool is_error() const { return !is_ok_; }
    const std::string& error() const { return error_; }
};

inline VoidResult Ok() {
    return VoidResult();
}

} // namespace rumina
```

```stmt.h
#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"

namespace rumina {

// 语句执行实现（供解释器内部使用）
namespace stmt {

void execute_stmt(Interpreter* interp, const Stmt* stmt);

} // namespace stmt
} // namespace rumina
```

```token.h
#pragma once

#include <string>
#include <variant>
#include <cstdint>

namespace rumina {

// Token类型
enum class TokenType {
    // 字面量
    Int,
    Float,
    Decimal,
    String,
    True,
    False,
    Null,

    // 标识符
    Ident,

    // 关键字
    Var,
    Let,
    BigInt,
    Struct,
    Func,
    Return,
    If,
    Else,
    While,
    For,
    Loop,
    Break,
    Continue,
    Include,
    Do,

    // 类型关键字
    TypeInt,
    TypeFloat,
    TypeBool,
    TypeString,
    TypeRational,
    TypeIrrational,
    TypeComplex,
    TypeArray,

    // 运算符
    Plus,    // +
    Minus,   // -
    Star,    // *
    Slash,   // /
    Percent, // %
    Caret,   // ^
    Bang,    // !

    // 比较运算符
    Equal,        // =
    EqualEqual,   // ==
    BangEqual,    // !=
    Greater,      // >
    GreaterEqual, // >=
    Less,         // <
    LessEqual,    // <=

    // 逻辑运算符
    And, // &&
    Or,  // ||

    // 分隔符
    Semicolon,   // ;
    Comma,       // ,
    Dot,         // .
    Colon,       // :
    DoubleColon, // ::
    Pipe,        // |
    PipeForward, // |>
    Backslash,   // \

    // 括号
    LParen,   // (
    RParen,   // )
    LBrace,   // {
    RBrace,   // }
    LBracket, // [
    RBracket, // ]

    // 特殊
    Arrow, // ->
    At,    // @
    Eof
};

// Token值类型
using TokenValue = std::variant<
    std::monostate,
    int64_t,
    double,
    std::string,
    bool
>;

// Token结构
struct Token {
    TokenType type;
    TokenValue value;
    size_t line;
    size_t column;

    Token(TokenType t, size_t l, size_t c) : type(t), line(l), column(c) {}
    
    template<typename T>
    Token(TokenType t, T v, size_t l, size_t c) 
        : type(t), value(v), line(l), column(c) {}

    bool operator==(const Token& other) const;
    std::string toString() const;
};

// 辅助函数
std::string tokenTypeToString(TokenType type);

} // namespace rumina
```

```value.h
#pragma once

#include "fwd.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <variant>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <gmpxx.h>

namespace rumina {

// 使用GMP的大整数和有理数
using BigInt = mpz_class;
using BigRational = mpq_class;

// 函数指针类型 - 已经在 fwd.h 中声明
// using NativeFunction = std::function<Value(const std::vector<Value>&)>;

// 无理数表示
class IrrationalValue {
public:
    enum class Kind {
        Sqrt,
        Root,
        Pi,
        E,
        Product,
        Sum
    };

private:
    Kind kind_ = Kind::Pi;
    std::shared_ptr<Value> sqrt_value_;
    uint32_t root_degree_ = 0;
    std::shared_ptr<Value> root_value_;
    std::shared_ptr<Value> product_coeff_;
    std::shared_ptr<IrrationalValue> product_irr_;
    std::shared_ptr<IrrationalValue> sum_left_;
    std::shared_ptr<IrrationalValue> sum_right_;

public:
    IrrationalValue() = default;
    IrrationalValue(const IrrationalValue&) = default;
    IrrationalValue& operator=(const IrrationalValue&) = default;
    
    static IrrationalValue makeSqrt(std::shared_ptr<Value> value);
    static IrrationalValue makeRoot(uint32_t degree, std::shared_ptr<Value> value);
    static IrrationalValue makePi();
    static IrrationalValue makeE();
    static IrrationalValue makeProduct(std::shared_ptr<Value> coeff, 
                                       std::shared_ptr<IrrationalValue> irr);
    static IrrationalValue makeSum(std::shared_ptr<IrrationalValue> left,
                                   std::shared_ptr<IrrationalValue> right);

    Kind getKind() const { return kind_; }
    const std::shared_ptr<Value>& getSqrtValue() const { return sqrt_value_; }
    uint32_t getRootDegree() const { return root_degree_; }
    const std::shared_ptr<Value>& getRootValue() const { return root_value_; }
    const std::shared_ptr<Value>& getProductCoeff() const { return product_coeff_; }
    const std::shared_ptr<IrrationalValue>& getProductIrr() const { return product_irr_; }
    const std::shared_ptr<IrrationalValue>& getSumLeft() const { return sum_left_; }
    const std::shared_ptr<IrrationalValue>& getSumRight() const { return sum_right_; }

    double toFloat() const;
    std::string toString() const;
    bool operator==(const IrrationalValue& other) const;
};

// Lamina运行时值类型
class Value {
public:
    enum class Type {
        Int, Float, BigInt, Rational, Irrational, Complex,
        Bool, String, Null, Array, Struct,
        Lambda, Function, Module, NativeFunction,
        CurriedFunction, MemoizedFunction
    };

    // 数据结构
    struct LambdaData {
        std::vector<std::string> params;
        std::shared_ptr<Stmt> body;
        std::shared_ptr<std::unordered_map<std::string, Value>> closure;
        LambdaData() = default;
        LambdaData(const LambdaData&) = default;
        LambdaData& operator=(const LambdaData&) = default;
    };

    struct FunctionData {
        std::string name;
        std::vector<std::string> params;
        std::shared_ptr<Stmt> body;
        std::vector<std::string> decorators;
        FunctionData() = default;
        FunctionData(const FunctionData&) = default;
        FunctionData& operator=(const FunctionData&) = default;
    };

    struct NativeFunctionData {
        std::string name;
        NativeFunction func;
        NativeFunctionData() = default;
        NativeFunctionData(const NativeFunctionData&) = default;
        NativeFunctionData& operator=(const NativeFunctionData&) = default;
    };

    struct CurriedFunctionData {
        std::shared_ptr<Value> original;
        std::vector<Value> collected_args;
        size_t total_params;
        CurriedFunctionData() = default;
        CurriedFunctionData(const CurriedFunctionData&) = default;
        CurriedFunctionData& operator=(const CurriedFunctionData&) = default;
    };

    struct MemoizedFunctionData {
        std::shared_ptr<Value> original;
        std::shared_ptr<std::unordered_map<std::string, Value>> cache;
        MemoizedFunctionData() = default;
        MemoizedFunctionData(const MemoizedFunctionData&) = default;
        MemoizedFunctionData& operator=(const MemoizedFunctionData&) = default;
    };

    template<typename T>
    const T* get() const {
        return std::get_if<T>(&data_);
    }

    template<typename T>
    T* get() {
        return std::get_if<T>(&data_);
    }

private:
    Type type_;
    
    using Data = std::variant<
        std::monostate,
        int64_t,
        double,
        bool,
        std::string,
        BigInt,
        BigRational,
        IrrationalValue,
        std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>>,
        std::shared_ptr<std::vector<Value>>,
        std::shared_ptr<std::unordered_map<std::string, Value>>,
        LambdaData,
        FunctionData,
        NativeFunctionData,
        CurriedFunctionData,
        MemoizedFunctionData
    >;
    
    Data data_;

public:
    // 构造函数
    Value() : type_(Type::Null), data_(std::monostate{}) {}
    
    // 基本类型构造函数
    Value(int64_t i) : type_(Type::Int), data_(i) {}
    Value(double f) : type_(Type::Float), data_(f) {}
    Value(bool b) : type_(Type::Bool), data_(b) {}
    Value(const std::string& s) : type_(Type::String), data_(s) {}
    Value(const char* s) : type_(Type::String), data_(std::string(s)) {}
    Value(const BigInt& bi) : type_(Type::BigInt), data_(bi) {}
    Value(const BigRational& br) : type_(Type::Rational), data_(br) {}
    Value(const IrrationalValue& irr) : type_(Type::Irrational), data_(irr) {}
    
    // 复数构造函数
    Value(std::shared_ptr<Value> re, std::shared_ptr<Value> im) 
        : type_(Type::Complex), data_(std::make_pair(re, im)) {}

    // 静态工厂方法（用于复杂类型）
    static Value makeArray(std::shared_ptr<std::vector<Value>> arr) {
        Value v;
        v.type_ = Type::Array;
        v.data_ = arr;
        return v;
    }
    
    static Value makeStruct(std::shared_ptr<std::unordered_map<std::string, Value>> s) {
        Value v;
        v.type_ = Type::Struct;
        v.data_ = s;
        return v;
    }
    
    static Value makeModule(std::shared_ptr<std::unordered_map<std::string, Value>> module) {
        Value v;
        v.type_ = Type::Module;
        v.data_ = module;
        return v;
    }
    
    static Value makeLambda(const LambdaData& data) {
        Value v;
        v.type_ = Type::Lambda;
        v.data_ = data;
        return v;
    }
    
    static Value makeFunction(const FunctionData& data) {
        Value v;
        v.type_ = Type::Function;
        v.data_ = data;
        return v;
    }
    
    static Value makeNativeFunction(const std::string& name, NativeFunction func) {
        Value v;
        v.type_ = Type::NativeFunction;
        v.data_ = NativeFunctionData{name, func};
        return v;
    }
    
    static Value makeCurriedFunction(std::shared_ptr<Value> original,
                                     std::vector<Value> args,
                                     size_t total) {
        Value v;
        v.type_ = Type::CurriedFunction;
        v.data_ = CurriedFunctionData{original, std::move(args), total};
        return v;
    }
    
    static Value makeMemoizedFunction(std::shared_ptr<Value> original) {
        Value v;
        v.type_ = Type::MemoizedFunction;
        v.data_ = MemoizedFunctionData{
            original,
            std::make_shared<std::unordered_map<std::string, Value>>()
        };
        return v;
    }

    // 拷贝和移动
    Value(const Value&) = default;
    Value(Value&&) = default;
    Value& operator=(const Value&) = default;
    Value& operator=(Value&&) = default;

    // 类型检查
    Type getType() const { return type_; }
    std::string typeName() const;

    // 值访问
    int64_t getInt() const { 
        if (type_ != Type::Int) throw std::runtime_error("Not an int");
        return std::get<int64_t>(data_); 
    }
    
    double getFloat() const { 
        if (type_ != Type::Float) throw std::runtime_error("Not a float");
        return std::get<double>(data_); 
    }
    
    bool getBool() const { 
        if (type_ != Type::Bool) throw std::runtime_error("Not a bool");
        return std::get<bool>(data_); 
    }
    
    const std::string& getString() const { 
        if (type_ != Type::String) throw std::runtime_error("Not a string");
        return std::get<std::string>(data_); 
    }
    
    const BigInt& getBigInt() const { 
        if (type_ != Type::BigInt) throw std::runtime_error("Not a bigint");
        return std::get<BigInt>(data_); 
    }
    
    const BigRational& getRational() const { 
        if (type_ != Type::Rational) throw std::runtime_error("Not a rational");
        return std::get<BigRational>(data_); 
    }
    
    const IrrationalValue& getIrrational() const { 
        if (type_ != Type::Irrational) throw std::runtime_error("Not an irrational");
        return std::get<IrrationalValue>(data_); 
    }

    std::shared_ptr<std::vector<Value>> getArray() const {
        if (type_ != Type::Array) throw std::runtime_error("Not an array");
        return std::get<std::shared_ptr<std::vector<Value>>>(data_);
    }

    std::shared_ptr<std::unordered_map<std::string, Value>> getStruct() const {
        if (type_ != Type::Struct && type_ != Type::Module) 
            throw std::runtime_error("Not a struct or module");
        return std::get<std::shared_ptr<std::unordered_map<std::string, Value>>>(data_);
    }

    std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>> getComplex() const {
        if (type_ != Type::Complex) throw std::runtime_error("Not a complex");
        return std::get<std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>>>(data_);
    }

    LambdaData getLambda() const {
        if (type_ != Type::Lambda) throw std::runtime_error("Not a lambda");
        return std::get<LambdaData>(data_);
    }

    FunctionData getFunction() const {
        if (type_ != Type::Function) throw std::runtime_error("Not a function");
        return std::get<FunctionData>(data_);
    }

    std::shared_ptr<std::unordered_map<std::string, Value>> getModule() const {
        if (type_ != Type::Module) throw std::runtime_error("Not a module");
        return std::get<std::shared_ptr<std::unordered_map<std::string, Value>>>(data_);
    }

    NativeFunctionData getNativeFunction() const {
        if (type_ != Type::NativeFunction) throw std::runtime_error("Not a native function");
        return std::get<NativeFunctionData>(data_);
    }

    CurriedFunctionData getCurriedFunction() const {
        if (type_ != Type::CurriedFunction) throw std::runtime_error("Not a curried function");
        return std::get<CurriedFunctionData>(data_);
    }

    MemoizedFunctionData getMemoizedFunction() const {
        if (type_ != Type::MemoizedFunction) throw std::runtime_error("Not a memoized function");
        return std::get<MemoizedFunctionData>(data_);
    }

    // 操作
    bool isTruthy() const;
    double toFloat() const;
    int64_t toInt() const;

    // 比较
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const { return !(*this == other); }

    // 字符串表示
    std::string toString() const;

    // 获取数据（用于value.cc中的辅助函数）
    const Data& getData() const { return data_; }
    Data& getData() { return data_; }
};

// 全局辅助函数
double irrationalToFloat(const IrrationalValue& irr);
std::string formatIrrational(const IrrationalValue& irr);

} // namespace rumina

// 哈希支持
namespace std {
    template<>
    struct hash<rumina::Value> {
        size_t operator()(const rumina::Value& v) const {
            return hash<string>()(v.toString());
        }
    };
}
```

```value_ops.h
#pragma once

#include "value.h"
#include "ast.h"
#include "result.h"
#include <memory>

namespace rumina {

// 并行幂运算阈值
constexpr uint32_t PARALLEL_POW_THRESHOLD = 10000;

// 二进制操作
Result<Value> value_binary_op(const Value& left, BinOp op, const Value& right);

// 一元操作
Result<Value> value_unary_op(UnaryOp op, const Value& val);

// 大数幂运算优化
BigInt bigint_pow_optimized(const BigInt& base, uint32_t exponent);

// 并行大数幂运算
BigInt bigint_pow_parallel(const BigInt& base, uint32_t exponent);

// 幂运算符号处理
Result<Value> compute_power(double base, double exponent);

// 无理数乘法
Result<Value> multiply_irrationals(const IrrationalValue& a, const IrrationalValue& b);

} // namespace rumina
```

```vm.h
#pragma once

#include "fwd.h"
#include "value.h"
#include "ast.h"
#include "result.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>

namespace rumina {

// 函数定义信息
struct FuncDefInfo {
    std::string name;
    std::vector<std::string> params;
    size_t body_start;
    size_t body_end;
    std::vector<std::string> decorators;
    
    FuncDefInfo() = default;
    FuncDefInfo(const FuncDefInfo& other) = default;
    FuncDefInfo& operator=(const FuncDefInfo& other) = default;
    FuncDefInfo(FuncDefInfo&& other) = default;
    FuncDefInfo& operator=(FuncDefInfo&& other) = default;
};

// Lambda信息
struct LambdaInfo {
    std::vector<std::string> params;
    size_t body_start;
    size_t body_end;
    
    LambdaInfo() = default;
    LambdaInfo(const LambdaInfo& other) = default;
    LambdaInfo& operator=(const LambdaInfo& other) = default;
    LambdaInfo(LambdaInfo&& other) = default;
    LambdaInfo& operator=(LambdaInfo&& other) = default;
};

// 操作码类型
enum class OpCodeType {
    PushConst, PushConstPooled, PushVar, PopVar, MarkImmutable, Dup, Pop,
    Add, Sub, Mul, Div, Mod, Pow, Neg, Factorial,
    Not, And, Or, Eq, Neq, Gt, Gte, Lt, Lte,
    Jump, JumpIfFalse, JumpIfTrue,
    CallVar, Call, CallMethod, Return,
    MakeArray, MakeStruct, Index, Member, IndexAssign, MemberAssign, MemberAssignVar,
    DefineFunc, MakeLambda,
    Break, Continue, Halt,
    ConvertType
};

// 操作码
struct OpCode {
    OpCodeType type;
    
    // 使用 union 和类型标记
    union Payload {
        Value value;
        size_t size;
        std::string str;
        std::pair<std::string, size_t> call_var;
        std::pair<std::string, std::string> member_assign;
        FuncDefInfo func_info;
        LambdaInfo lambda_info;
        DeclaredType decl_type;
        
        Payload() {}
        ~Payload() {}
    } payload;
    
    enum PayloadType {
        PAYLOAD_NONE,
        PAYLOAD_VALUE,
        PAYLOAD_SIZE,
        PAYLOAD_STRING,
        PAYLOAD_CALL_VAR,
        PAYLOAD_MEMBER_ASSIGN,
        PAYLOAD_FUNC_INFO,
        PAYLOAD_LAMBDA_INFO,
        PAYLOAD_DECL_TYPE
    } payload_type;

    // 构造函数 - 明确区分类型
    explicit OpCode(OpCodeType t) : type(t), payload_type(PAYLOAD_NONE) {}
    
    explicit OpCode(OpCodeType t, const Value& v) : type(t), payload_type(PAYLOAD_VALUE) {
        new (&payload.value) Value(v);
    }
    
    explicit OpCode(OpCodeType t, size_t n) : type(t), payload_type(PAYLOAD_SIZE) {
        payload.size = n;
    }
    
    explicit OpCode(OpCodeType t, const char* s) : type(t), payload_type(PAYLOAD_STRING) {
        new (&payload.str) std::string(s);
    }
    
    explicit OpCode(OpCodeType t, const std::string& s) : type(t), payload_type(PAYLOAD_STRING) {
        new (&payload.str) std::string(s);
    }
    
    explicit OpCode(OpCodeType t, const std::pair<std::string, size_t>& p) 
        : type(t), payload_type(PAYLOAD_CALL_VAR) {
        new (&payload.call_var) std::pair<std::string, size_t>(p);
    }
    
    explicit OpCode(OpCodeType t, const std::pair<std::string, std::string>& p) 
        : type(t), payload_type(PAYLOAD_MEMBER_ASSIGN) {
        new (&payload.member_assign) std::pair<std::string, std::string>(p);
    }
    
    explicit OpCode(OpCodeType t, const FuncDefInfo& info) 
        : type(t), payload_type(PAYLOAD_FUNC_INFO) {
        new (&payload.func_info) FuncDefInfo(info);
    }
    
    explicit OpCode(OpCodeType t, const LambdaInfo& info) 
        : type(t), payload_type(PAYLOAD_LAMBDA_INFO) {
        new (&payload.lambda_info) LambdaInfo(info);
    }
    
    explicit OpCode(OpCodeType t, DeclaredType dt) 
        : type(t), payload_type(PAYLOAD_DECL_TYPE) {
        payload.decl_type = dt;
    }
    
    // 拷贝构造
    OpCode(const OpCode& other) : type(other.type), payload_type(other.payload_type) {
        switch (payload_type) {
            case PAYLOAD_VALUE: new (&payload.value) Value(other.payload.value); break;
            case PAYLOAD_SIZE: payload.size = other.payload.size; break;
            case PAYLOAD_STRING: new (&payload.str) std::string(other.payload.str); break;
            case PAYLOAD_CALL_VAR: new (&payload.call_var) std::pair<std::string, size_t>(other.payload.call_var); break;
            case PAYLOAD_MEMBER_ASSIGN: new (&payload.member_assign) std::pair<std::string, std::string>(other.payload.member_assign); break;
            case PAYLOAD_FUNC_INFO: new (&payload.func_info) FuncDefInfo(other.payload.func_info); break;
            case PAYLOAD_LAMBDA_INFO: new (&payload.lambda_info) LambdaInfo(other.payload.lambda_info); break;
            case PAYLOAD_DECL_TYPE: payload.decl_type = other.payload.decl_type; break;
            default: break;
        }
    }
    
    // 赋值操作符
    OpCode& operator=(const OpCode& other) {
        if (this != &other) {
            this->~OpCode();
            new (this) OpCode(other);
        }
        return *this;
    }
    
    // 移动构造
    OpCode(OpCode&& other) : type(other.type), payload_type(other.payload_type) {
        switch (payload_type) {
            case PAYLOAD_VALUE: new (&payload.value) Value(std::move(other.payload.value)); break;
            case PAYLOAD_SIZE: payload.size = other.payload.size; break;
            case PAYLOAD_STRING: new (&payload.str) std::string(std::move(other.payload.str)); break;
            case PAYLOAD_CALL_VAR: new (&payload.call_var) std::pair<std::string, size_t>(std::move(other.payload.call_var)); break;
            case PAYLOAD_MEMBER_ASSIGN: new (&payload.member_assign) std::pair<std::string, std::string>(std::move(other.payload.member_assign)); break;
            case PAYLOAD_FUNC_INFO: new (&payload.func_info) FuncDefInfo(std::move(other.payload.func_info)); break;
            case PAYLOAD_LAMBDA_INFO: new (&payload.lambda_info) LambdaInfo(std::move(other.payload.lambda_info)); break;
            case PAYLOAD_DECL_TYPE: payload.decl_type = other.payload.decl_type; break;
            default: break;
        }
        other.payload_type = PAYLOAD_NONE;
    }
    
    // 移动赋值
    OpCode& operator=(OpCode&& other) {
        if (this != &other) {
            this->~OpCode();
            new (this) OpCode(std::move(other));
        }
        return *this;
    }
    
    ~OpCode() {
        switch (payload_type) {
            case PAYLOAD_VALUE: payload.value.~Value(); break;
            case PAYLOAD_STRING: payload.str.~basic_string(); break;
            case PAYLOAD_CALL_VAR: payload.call_var.~pair(); break;
            case PAYLOAD_MEMBER_ASSIGN: payload.member_assign.~pair(); break;
            case PAYLOAD_FUNC_INFO: payload.func_info.~FuncDefInfo(); break;
            case PAYLOAD_LAMBDA_INFO: payload.lambda_info.~LambdaInfo(); break;
            default: break;
        }
    }
};

// 字节码块
class ByteCode {
public:
    ByteCode() = default;
    ByteCode(const ByteCode&) = delete; // 禁止拷贝
    ByteCode& operator=(const ByteCode&) = delete;
    ByteCode(ByteCode&&) = default;
    ByteCode& operator=(ByteCode&&) = default;

    void emit(OpCode op, std::optional<size_t> line);
    size_t currentAddress() const;
    void patchJump(size_t address, size_t target);
    
    size_t addConstant(const Value& value);
    
    std::string serialize() const;
    static ByteCode deserialize(const std::string& input);

    const std::vector<OpCode>& getInstructions() const { return instructions_; }
    std::vector<OpCode>& getInstructions() { return instructions_; }
    const std::vector<std::optional<size_t>>& getLineNumbers() const { return line_numbers_; }
    std::vector<std::optional<size_t>>& getLineNumbers() { return line_numbers_; }
    const std::vector<Value>& getConstants() const { return constants_; }
    std::vector<Value>& getConstants() { return constants_; }

private:
    std::vector<OpCode> instructions_;
    std::vector<std::optional<size_t>> line_numbers_;
    std::vector<Value> constants_;
    static bool valuesEqual(const Value& a, const Value& b);
    
    // 不使用 std::unordered_map 来避免哈希问题，改用线性搜索
    // 因为常量池通常很小
};

// 调用帧
struct CallFrame {
    size_t return_address;
    size_t base_pointer;
    std::string function_name;
    std::unordered_map<std::string, Value> locals;
    std::unordered_set<std::string> immutable_locals;
};

// 虚拟机
class VM {
public:
    explicit VM(std::shared_ptr<std::unordered_map<std::string, Value>> globals);

    void load(ByteCode bytecode);
    Result<std::optional<Value>> run();

    std::pair<size_t, size_t> getCacheStats() const;

private:
    ByteCode bytecode_;
    size_t ip_ = 0;
    std::vector<Value> stack_;
    std::vector<CallFrame> call_stack_;
    
    std::shared_ptr<std::unordered_map<std::string, Value>> globals_;
    std::unordered_map<std::string, Value> locals_;
    std::unordered_set<std::string> immutable_globals_;
    std::unordered_set<std::string> immutable_locals_;
    
    std::vector<std::pair<size_t, size_t>> loop_stack_;
    std::unordered_map<std::string, FuncDefInfo> functions_;
    
    struct InlineCache {
        std::string member;
        size_t hits = 0;
        size_t misses = 0;
    };
    std::unordered_map<size_t, InlineCache> member_cache_;
    
    bool halted_ = false;
    size_t recursion_depth_ = 0;
    static constexpr size_t MAX_RECURSION_DEPTH = 4000;

    void executeInstructionAt(size_t ip);
    
    template<typename F>
    void binaryOp(F&& f);
    
    Value getVariable(const std::string& name) const;
    void setVariable(const std::string& name, const Value& value);
    void ensureMutable(const std::string& name) const;
    void setVariableChecked(const std::string& name, const Value& value);
    
    Value convertToType(const Value& val, DeclaredType dtype) const;
};

} // namespace rumina
```

```vm_ops.h
#pragma once

#include "value.h"
#include "ast.h"
#include "result.h"
#include <functional>

namespace rumina {

// VM操作接口类
class VMOperations {
public:
    virtual ~VMOperations() = default;

    // 算术运算
    virtual Result<Value> vm_add(const Value& other) const = 0;
    virtual Result<Value> vm_sub(const Value& other) const = 0;
    virtual Result<Value> vm_mul(const Value& other) const = 0;
    virtual Result<Value> vm_div(const Value& other) const = 0;
    virtual Result<Value> vm_mod(const Value& other) const = 0;
    virtual Result<Value> vm_pow(const Value& other) const = 0;
    
    // 一元运算
    virtual Result<Value> vm_neg() const = 0;
    virtual Result<Value> vm_not() const = 0;
    virtual Result<Value> vm_factorial() const = 0;

    // 比较运算
    virtual Result<Value> vm_eq(const Value& other) const = 0;
    virtual Result<Value> vm_neq(const Value& other) const = 0;
    virtual Result<Value> vm_gt(const Value& other) const = 0;
    virtual Result<Value> vm_gte(const Value& other) const = 0;
    virtual Result<Value> vm_lt(const Value& other) const = 0;
    virtual Result<Value> vm_lte(const Value& other) const = 0;

    // 逻辑运算
    virtual Result<Value> vm_and(const Value& other) const = 0;
    virtual Result<Value> vm_or(const Value& other) const = 0;
};

// Value类的VM操作实现
class ValueVMOps : public VMOperations {
private:
    const Value& value_;

public:
    explicit ValueVMOps(const Value& v) : value_(v) {}

    Result<Value> vm_add(const Value& other) const override;
    Result<Value> vm_sub(const Value& other) const override;
    Result<Value> vm_mul(const Value& other) const override;
    Result<Value> vm_div(const Value& other) const override;
    Result<Value> vm_mod(const Value& other) const override;
    Result<Value> vm_pow(const Value& other) const override;

    Result<Value> vm_neg() const override;
    Result<Value> vm_not() const override;
    Result<Value> vm_factorial() const override;

    Result<Value> vm_eq(const Value& other) const override;
    Result<Value> vm_neq(const Value& other) const override;
    Result<Value> vm_gt(const Value& other) const override;
    Result<Value> vm_gte(const Value& other) const override;
    Result<Value> vm_lt(const Value& other) const override;
    Result<Value> vm_lte(const Value& other) const override;

    Result<Value> vm_and(const Value& other) const override;
    Result<Value> vm_or(const Value& other) const override;
};

// 辅助函数
inline ValueVMOps get_vm_ops(const Value& value) {
    return ValueVMOps(value);
}

} // namespace rumina
```

