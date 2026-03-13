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
