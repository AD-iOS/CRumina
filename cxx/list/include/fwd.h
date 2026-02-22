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
