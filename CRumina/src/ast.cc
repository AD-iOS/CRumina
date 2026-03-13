#include <ast.h>

#include <sstream>

namespace rumina {

// IntExpr::toString
std::string IntExpr::toString() const {
    return std::to_string(value);
}

// FloatExpr::toString
std::string FloatExpr::toString() const {
    return std::to_string(value);
}

// StringExpr::toString
std::string StringExpr::toString() const {
    return "\"" + value + "\"";
}

// BoolExpr::toString
std::string BoolExpr::toString() const {
    return value ? "true" : "false";
}

// NullExpr::toString
std::string NullExpr::toString() const {
    return "null";
}

// IdentExpr::toString
std::string IdentExpr::toString() const {
    return name;
}

// BinaryExpr::toString
std::string BinaryExpr::toString() const {
    return "(" + left->toString() + " " + binOpToString(op) + " " + right->toString() + ")";
}

// UnaryExpr::toString
std::string UnaryExpr::toString() const {
    return unaryOpToString(op) + expr->toString();
}

// ArrayExpr::toString
std::string ArrayExpr::toString() const {
    std::string result = "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) result += ", ";
        result += elements[i]->toString();
    }
    result += "]";
    return result;
}

// StructExpr::toString
std::string StructExpr::toString() const {
    std::string result = "{";
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) result += ", ";
        result += fields[i].first + " = " + fields[i].second->toString();
    }
    result += "}";
    return result;
}

// CallExpr::toString
std::string CallExpr::toString() const {
    std::string result = func->toString() + "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) result += ", ";
        result += args[i]->toString();
    }
    result += ")";
    return result;
}

// MemberExpr::toString
std::string MemberExpr::toString() const {
    return object->toString() + "." + member;
}

// IndexExpr::toString
std::string IndexExpr::toString() const {
    return object->toString() + "[" + index->toString() + "]";
}

// LambdaExpr::toString
std::string LambdaExpr::toString() const {
    std::string result = "|";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) result += ", ";
        result += params[i];
    }
    result += "| " + body->toString();
    return result;
}

// NamespaceExpr::toString
std::string NamespaceExpr::toString() const {
    return module + "::" + name;
}

// VarDeclStmt::toString
std::string VarDeclStmt::toString() const {
    std::string result = "var " + name;
    if (declared_type.has_value()) {
        switch (declared_type.value()) {
            case DeclaredType::Int: result += ": int"; break;
            case DeclaredType::Float: result += ": float"; break;
            case DeclaredType::Bool: result += ": bool"; break;
            case DeclaredType::String: result += ": string"; break;
            case DeclaredType::Rational: result += ": rational"; break;
            case DeclaredType::Irrational: result += ": irrational"; break;
            case DeclaredType::Complex: result += ": complex"; break;
            case DeclaredType::Array: result += ": array"; break;
            case DeclaredType::BigInt: result += ": bigint"; break;
        }
    } else if (is_bigint) {
        result += ": bigint";
    }
    result += " = " + value->toString() + ";";
    return result;
}

// LetDeclStmt::toString
std::string LetDeclStmt::toString() const {
    std::string result = "let " + name;
    if (declared_type.has_value()) {
        switch (declared_type.value()) {
            case DeclaredType::Int: result += ": int"; break;
            case DeclaredType::Float: result += ": float"; break;
            case DeclaredType::Bool: result += ": bool"; break;
            case DeclaredType::String: result += ": string"; break;
            case DeclaredType::Rational: result += ": rational"; break;
            case DeclaredType::Irrational: result += ": irrational"; break;
            case DeclaredType::Complex: result += ": complex"; break;
            case DeclaredType::Array: result += ": array"; break;
            case DeclaredType::BigInt: result += ": bigint"; break;
        }
    } else if (is_bigint) {
        result += ": bigint";
    }
    result += " = " + value->toString() + ";";
    return result;
}

// AssignStmt::toString
std::string AssignStmt::toString() const {
    return name + " = " + value->toString() + ";";
}

// MemberAssignStmt::toString
std::string MemberAssignStmt::toString() const {
    return object->toString() + "." + member + " = " + value->toString() + ";";
}

// ExprStmt::toString
std::string ExprStmt::toString() const {
    return expr->toString() + ";";
}

// FuncDefStmt::toString
std::string FuncDefStmt::toString() const {
    std::string result = "func " + name + "(";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) result += ", ";
        result += params[i];
    }
    result += ") {\n";
    for (const auto& stmt : body) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

// ReturnStmt::toString
std::string ReturnStmt::toString() const {
    if (expr.has_value()) {
        return "return " + expr.value()->toString() + ";";
    } else {
        return "return;";
    }
}

// IfStmt::toString
std::string IfStmt::toString() const {
    std::string result = "if (" + condition->toString() + ") {\n";
    for (const auto& stmt : then_branch) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    if (else_branch.has_value()) {
        result += " else {\n";
        for (const auto& stmt : else_branch.value()) {
            result += "  " + stmt->toString() + "\n";
        }
        result += "}";
    }
    return result;
}

// WhileStmt::toString
std::string WhileStmt::toString() const {
    std::string result = "while (" + condition->toString() + ") {\n";
    for (const auto& stmt : body) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

// ForStmt::toString
std::string ForStmt::toString() const {
    std::string result = "for (";
    if (init.has_value()) {
        result += init.value()->toString();
    } else {
        result += ";";
    }
    result += " ";
    if (condition.has_value()) {
        result += condition.value()->toString();
    }
    result += "; ";
    if (update.has_value()) {
        result += update.value()->toString();
    }
    result += ") {\n";
    for (const auto& stmt : body) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

// LoopStmt::toString
std::string LoopStmt::toString() const {
    std::string result = "loop {\n";
    for (const auto& stmt : body) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

// BreakStmt::toString
std::string BreakStmt::toString() const {
    return "break;";
}

// ContinueStmt::toString
std::string ContinueStmt::toString() const {
    return "continue;";
}

// IncludeStmt::toString
std::string IncludeStmt::toString() const {
    return "include \"" + path + "\";";
}

// BlockStmt::toString
std::string BlockStmt::toString() const {
    std::string result = "{\n";
    for (const auto& stmt : statements) {
        result += "  " + stmt->toString() + "\n";
    }
    result += "}";
    return result;
}

// EmptyStmt::toString
std::string EmptyStmt::toString() const {
    return ";";
}

// binOpToString
std::string binOpToString(BinOp op) {
    switch (op) {
        case BinOp::Add: return "+";
        case BinOp::Sub: return "-";
        case BinOp::Mul: return "*";
        case BinOp::Div: return "/";
        case BinOp::Mod: return "%";
        case BinOp::Pow: return "^";
        case BinOp::Equal: return "==";
        case BinOp::NotEqual: return "!=";
        case BinOp::Greater: return ">";
        case BinOp::GreaterEq: return ">=";
        case BinOp::Less: return "<";
        case BinOp::LessEq: return "<=";
        case BinOp::And: return "&&";
        case BinOp::Or: return "||";
    }
    return "unknown";
}

// unaryOpToString
std::string unaryOpToString(UnaryOp op) {
    switch (op) {
        case UnaryOp::Neg: return "-";
        case UnaryOp::Not: return "!";
        case UnaryOp::Factorial: return "!";
    }
    return "unknown";
}

} // namespace rumina
