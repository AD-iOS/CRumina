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
