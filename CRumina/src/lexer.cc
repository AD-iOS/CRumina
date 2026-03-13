#include <lexer.h>

#include <cctype>
#include <sstream>

namespace rumina {

Lexer::Lexer(std::string input) 
    : input_(std::move(input)) {
    if (!input_.empty()) {
        current_char_ = input_[0];
    }
}

void Lexer::advance() {
    if (position_ < input_.size()) {
        if (input_[position_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        position_++;
        current_char_ = position_ < input_.size() 
            ? std::optional<char>(input_[position_]) 
            : std::nullopt;
    }
}

std::optional<char> Lexer::peek() const {
    return position_ + 1 < input_.size() 
        ? std::optional<char>(input_[position_ + 1]) 
        : std::nullopt;
}

std::optional<char> Lexer::peekN(size_t n) const {
    return position_ + n < input_.size() 
        ? std::optional<char>(input_[position_ + n]) 
        : std::nullopt;
}

void Lexer::skipWhitespace() {
    while (current_char_.has_value() && 
           std::isspace(*current_char_) && 
           *current_char_ != '\\') {
        advance();
    }
}

void Lexer::skipComment() {
    if (current_char_ == '#' && !(peek() == '#' && peekN(2) == '#')) {
        while (current_char_.has_value() && *current_char_ != '\n') {
            advance();
        }
        if (current_char_ == '\n') {
            advance();
        }
        return;
    }

    if (current_char_ == '#' && peek() == '#' && peekN(2) == '#') {
        advance();
        advance();
        advance();

        while (current_char_.has_value()) {
            if (current_char_ == '#' && peek() == '#' && peekN(2) == '#') {
                advance();
                advance();
                advance();
                break;
            }
            advance();
        }
        return;
    }

    if (current_char_ == '/' && peek() == '/') {
        while (current_char_.has_value() && *current_char_ != '\n') {
            advance();
        }
        if (current_char_ == '\n') {
            advance();
        }
        return;
    }

    if (current_char_ == '/' && peek() == '*') {
        advance();
        advance();

        while (current_char_.has_value()) {
            if (current_char_ == '*' && peek() == '/') {
                advance();
                advance();
                break;
            }
            advance();
        }
    }
}

Token Lexer::readNumber() {
    std::string num_str;
    bool is_float = false;

    while (current_char_.has_value()) {
        char ch = *current_char_;
        if (std::isdigit(ch)) {
            num_str.push_back(ch);
            advance();
        } else if (ch == '.' && !is_float && peek().has_value() && 
                   std::isdigit(*peek())) {
            is_float = true;
            num_str.push_back(ch);
            advance();
        } else {
            break;
        }
    }

    if (is_float) {
        return Token(TokenType::Decimal, num_str, line_, column_ - num_str.length());
    } else {
        int64_t val = std::stoll(num_str);
        return Token(TokenType::Int, val, line_, column_ - num_str.length());
    }
}

Token Lexer::readString() {
    advance();
    std::string str;
    size_t start_line = line_;
    size_t start_col = column_ - 1;

    while (current_char_.has_value()) {
        char ch = *current_char_;
        if (ch == '"') {
            advance();
            break;
        } else if (ch == '\\') {
            advance();
            if (current_char_.has_value()) {
                char escaped = *current_char_;
                switch (escaped) {
                    case 'n': str.push_back('\n'); break;
                    case 't': str.push_back('\t'); break;
                    case 'r': str.push_back('\r'); break;
                    case '\\': str.push_back('\\'); break;
                    case '"': str.push_back('"'); break;
                    case '\'': str.push_back('\''); break;
                    default:
                        str.push_back('\\');
                        str.push_back(escaped);
                        break;
                }
                advance();
            }
        } else {
            str.push_back(ch);
            advance();
        }
    }

    return Token(TokenType::String, str, start_line, start_col);
}

Token Lexer::readSingleString() {
    advance();
    std::string str;
    size_t start_line = line_;
    size_t start_col = column_ - 1;

    while (current_char_.has_value()) {
        char ch = *current_char_;
        if (ch == '\'') {
            advance();
            break;
        } else if (ch == '\\') {
            advance();
            if (current_char_.has_value()) {
                char escaped = *current_char_;
                switch (escaped) {
                    case 'n': str.push_back('\n'); break;
                    case 't': str.push_back('\t'); break;
                    case 'r': str.push_back('\r'); break;
                    case '\\': str.push_back('\\'); break;
                    case '"': str.push_back('"'); break;
                    case '\'': str.push_back('\''); break;
                    default:
                        str.push_back('\\');
                        str.push_back(escaped);
                        break;
                }
                advance();
            }
        } else {
            str.push_back(ch);
            advance();
        }
    }

    return Token(TokenType::String, str, start_line, start_col);
}

Token Lexer::readIdentifier() {
    std::string ident;
    size_t start_line = line_;
    size_t start_col = column_;

    while (current_char_.has_value() && 
           (std::isalnum(*current_char_) || *current_char_ == '_')) {
        ident.push_back(*current_char_);
        advance();
    }

    if (ident == "var") return Token(TokenType::Var, start_line, start_col);
    if (ident == "let") return Token(TokenType::Let, start_line, start_col);
    if (ident == "bigint") return Token(TokenType::BigInt, start_line, start_col);
    if (ident == "struct") return Token(TokenType::Struct, start_line, start_col);
    if (ident == "func") return Token(TokenType::Func, start_line, start_col);
    if (ident == "return") return Token(TokenType::Return, start_line, start_col);
    if (ident == "if") return Token(TokenType::If, start_line, start_col);
    if (ident == "else") return Token(TokenType::Else, start_line, start_col);
    if (ident == "while") return Token(TokenType::While, start_line, start_col);
    if (ident == "for") return Token(TokenType::For, start_line, start_col);
    if (ident == "loop") return Token(TokenType::Loop, start_line, start_col);
    if (ident == "break") return Token(TokenType::Break, start_line, start_col);
    if (ident == "continue") return Token(TokenType::Continue, start_line, start_col);
    if (ident == "include") return Token(TokenType::Include, start_line, start_col);
    if (ident == "do") return Token(TokenType::Do, start_line, start_col);
    if (ident == "true") return Token(TokenType::True, true, start_line, start_col);
    if (ident == "false") return Token(TokenType::False, false, start_line, start_col);
    if (ident == "null") return Token(TokenType::Null, start_line, start_col);
    
    if (ident == "int") return Token(TokenType::TypeInt, start_line, start_col);
    if (ident == "float") return Token(TokenType::TypeFloat, start_line, start_col);
    if (ident == "bool") return Token(TokenType::TypeBool, start_line, start_col);
    if (ident == "string") return Token(TokenType::TypeString, start_line, start_col);
    if (ident == "rational") return Token(TokenType::TypeRational, start_line, start_col);
    if (ident == "irrational") return Token(TokenType::TypeIrrational, start_line, start_col);
    if (ident == "complex") return Token(TokenType::TypeComplex, start_line, start_col);
    if (ident == "array") return Token(TokenType::TypeArray, start_line, start_col);

    return Token(TokenType::Ident, ident, start_line, start_col);
}

Token Lexer::nextToken() {
    while (true) {
        skipWhitespace();

        if ((current_char_ == '/' && (peek() == '/' || peek() == '*')) ||
            current_char_ == '#') {
            skipComment();
            continue;
        }

        if (current_char_ == '\\') {
            auto next = peek();
            if (next == '\n' || next == '\r') {
                advance();
                if (current_char_ == '\r') advance();
                if (current_char_ == '\n') advance();
                continue;
            }
        }

        break;
    }

    if (!current_char_.has_value()) {
        return Token(TokenType::Eof, line_, column_);
    }

    char ch = *current_char_;
    size_t line = line_;
    size_t col = column_;

    switch (ch) {
        case '+': advance(); return Token(TokenType::Plus, line, col);
        case '-': 
            advance();
            if (current_char_ == '>') {
                advance();
                return Token(TokenType::Arrow, line, col);
            }
            return Token(TokenType::Minus, line, col);
        case '*': advance(); return Token(TokenType::Star, line, col);
        case '/': advance(); return Token(TokenType::Slash, line, col);
        case '%': advance(); return Token(TokenType::Percent, line, col);
        case '^': advance(); return Token(TokenType::Caret, line, col);
        case '!':
            advance();
            if (current_char_ == '=') {
                advance();
                return Token(TokenType::BangEqual, line, col);
            }
            return Token(TokenType::Bang, line, col);
        case '=':
            advance();
            if (current_char_ == '=') {
                advance();
                return Token(TokenType::EqualEqual, line, col);
            }
            return Token(TokenType::Equal, line, col);
        case '>':
            advance();
            if (current_char_ == '=') {
                advance();
                return Token(TokenType::GreaterEqual, line, col);
            }
            return Token(TokenType::Greater, line, col);
        case '<':
            advance();
            if (current_char_ == '=') {
                advance();
                return Token(TokenType::LessEqual, line, col);
            }
            return Token(TokenType::Less, line, col);
        case '&':
            advance();
            if (current_char_ == '&') {
                advance();
                return Token(TokenType::And, line, col);
            }
            throw std::runtime_error("Lexer error: Expected '&' after '&'");
        case '|':
            advance();
            if (current_char_ == '|') {
                advance();
                return Token(TokenType::Or, line, col);
            } else if (current_char_ == '>') {
                advance();
                return Token(TokenType::PipeForward, line, col);
            }
            return Token(TokenType::Pipe, line, col);
        case ';': advance(); return Token(TokenType::Semicolon, line, col);
        case ',': advance(); return Token(TokenType::Comma, line, col);
        case '.': advance(); return Token(TokenType::Dot, line, col);
        case ':':
            advance();
            if (current_char_ == ':') {
                advance();
                return Token(TokenType::DoubleColon, line, col);
            }
            return Token(TokenType::Colon, line, col);
        case '\\': advance(); return Token(TokenType::Backslash, line, col);
        case '@': advance(); return Token(TokenType::At, line, col);
        case '(': advance(); return Token(TokenType::LParen, line, col);
        case ')': advance(); return Token(TokenType::RParen, line, col);
        case '{': advance(); return Token(TokenType::LBrace, line, col);
        case '}': advance(); return Token(TokenType::RBrace, line, col);
        case '[': advance(); return Token(TokenType::LBracket, line, col);
        case ']': advance(); return Token(TokenType::RBracket, line, col);
        case '\'': return readSingleString();
        case '"': return readString();
        default:
            if (std::isdigit(ch)) {
                return readNumber();
            } else if (std::isalpha(ch) || ch == '_') {
                return readIdentifier();
            }
            throw std::runtime_error("Lexer error: Unexpected character");
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        auto token = nextToken();
        tokens.push_back(token);
        if (token.type == TokenType::Eof) {
            break;
        }
    }
    return tokens;
}

} // namespace rumina
