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
