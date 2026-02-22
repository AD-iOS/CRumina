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
