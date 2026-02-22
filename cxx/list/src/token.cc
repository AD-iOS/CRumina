#include <token.h>

namespace rumina {

bool Token::operator==(const Token& other) const {
    if (type != other.type) return false;
    
    switch (type) {
        case TokenType::Int:
            return std::get<int64_t>(value) == std::get<int64_t>(other.value);
        case TokenType::Float:
            return std::get<double>(value) == std::get<double>(other.value);
        case TokenType::Decimal:
        case TokenType::String:
        case TokenType::Ident:
            return std::get<std::string>(value) == std::get<std::string>(other.value);
        case TokenType::True:
        case TokenType::False:
            return std::get<bool>(value) == std::get<bool>(other.value);
        default:
            return true;
    }
}

std::string Token::toString() const {
    switch (type) {
        case TokenType::Int:
            return std::to_string(std::get<int64_t>(value));
        case TokenType::Float:
            return std::to_string(std::get<double>(value));
        case TokenType::Decimal:
            return std::get<std::string>(value);
        case TokenType::String:
            return "\"" + std::get<std::string>(value) + "\"";
        case TokenType::Ident:
            return std::get<std::string>(value);
        case TokenType::True:
            return "true";
        case TokenType::False:
            return "false";
        case TokenType::Null:
            return "null";
        case TokenType::Var:
            return "var";
        case TokenType::Let:
            return "let";
        case TokenType::BigInt:
            return "bigint";
        case TokenType::Struct:
            return "struct";
        case TokenType::Func:
            return "func";
        case TokenType::Return:
            return "return";
        case TokenType::If:
            return "if";
        case TokenType::Else:
            return "else";
        case TokenType::While:
            return "while";
        case TokenType::For:
            return "for";
        case TokenType::Loop:
            return "loop";
        case TokenType::Break:
            return "break";
        case TokenType::Continue:
            return "continue";
        case TokenType::Include:
            return "include";
        case TokenType::Do:
            return "do";
        case TokenType::TypeInt:
            return "int";
        case TokenType::TypeFloat:
            return "float";
        case TokenType::TypeBool:
            return "bool";
        case TokenType::TypeString:
            return "string";
        case TokenType::TypeRational:
            return "rational";
        case TokenType::TypeIrrational:
            return "irrational";
        case TokenType::TypeComplex:
            return "complex";
        case TokenType::TypeArray:
            return "array";
        case TokenType::Plus:
            return "+";
        case TokenType::Minus:
            return "-";
        case TokenType::Star:
            return "*";
        case TokenType::Slash:
            return "/";
        case TokenType::Percent:
            return "%";
        case TokenType::Caret:
            return "^";
        case TokenType::Bang:
            return "!";
        case TokenType::Equal:
            return "=";
        case TokenType::EqualEqual:
            return "==";
        case TokenType::BangEqual:
            return "!=";
        case TokenType::Greater:
            return ">";
        case TokenType::GreaterEqual:
            return ">=";
        case TokenType::Less:
            return "<";
        case TokenType::LessEqual:
            return "<=";
        case TokenType::And:
            return "&&";
        case TokenType::Or:
            return "||";
        case TokenType::Semicolon:
            return ";";
        case TokenType::Comma:
            return ",";
        case TokenType::Dot:
            return ".";
        case TokenType::Colon:
            return ":";
        case TokenType::DoubleColon:
            return "::";
        case TokenType::Pipe:
            return "|";
        case TokenType::PipeForward:
            return "|>";
        case TokenType::Backslash:
            return "\\";
        case TokenType::LParen:
            return "(";
        case TokenType::RParen:
            return ")";
        case TokenType::LBrace:
            return "{";
        case TokenType::RBrace:
            return "}";
        case TokenType::LBracket:
            return "[";
        case TokenType::RBracket:
            return "]";
        case TokenType::Arrow:
            return "->";
        case TokenType::At:
            return "@";
        case TokenType::Eof:
            return "EOF";
    }
    return "Unknown";
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Int: return "Int";
        case TokenType::Float: return "Float";
        case TokenType::Decimal: return "Decimal";
        case TokenType::String: return "String";
        case TokenType::Ident: return "Ident";
        case TokenType::True: return "True";
        case TokenType::False: return "False";
        case TokenType::Null: return "Null";
        case TokenType::Var: return "Var";
        case TokenType::Let: return "Let";
        case TokenType::BigInt: return "BigInt";
        case TokenType::Struct: return "Struct";
        case TokenType::Func: return "Func";
        case TokenType::Return: return "Return";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::While: return "While";
        case TokenType::For: return "For";
        case TokenType::Loop: return "Loop";
        case TokenType::Break: return "Break";
        case TokenType::Continue: return "Continue";
        case TokenType::Include: return "Include";
        case TokenType::Do: return "Do";
        case TokenType::TypeInt: return "TypeInt";
        case TokenType::TypeFloat: return "TypeFloat";
        case TokenType::TypeBool: return "TypeBool";
        case TokenType::TypeString: return "TypeString";
        case TokenType::TypeRational: return "TypeRational";
        case TokenType::TypeIrrational: return "TypeIrrational";
        case TokenType::TypeComplex: return "TypeComplex";
        case TokenType::TypeArray: return "TypeArray";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Percent: return "Percent";
        case TokenType::Caret: return "Caret";
        case TokenType::Bang: return "Bang";
        case TokenType::Equal: return "Equal";
        case TokenType::EqualEqual: return "EqualEqual";
        case TokenType::BangEqual: return "BangEqual";
        case TokenType::Greater: return "Greater";
        case TokenType::GreaterEqual: return "GreaterEqual";
        case TokenType::Less: return "Less";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::And: return "And";
        case TokenType::Or: return "Or";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Comma: return "Comma";
        case TokenType::Dot: return "Dot";
        case TokenType::Colon: return "Colon";
        case TokenType::DoubleColon: return "DoubleColon";
        case TokenType::Pipe: return "Pipe";
        case TokenType::PipeForward: return "PipeForward";
        case TokenType::Backslash: return "Backslash";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::LBrace: return "LBrace";
        case TokenType::RBrace: return "RBrace";
        case TokenType::LBracket: return "LBracket";
        case TokenType::RBracket: return "RBracket";
        case TokenType::Arrow: return "Arrow";
        case TokenType::At: return "At";
        case TokenType::Eof: return "Eof";
    }
    return "Unknown";
}

} // namespace rumina
