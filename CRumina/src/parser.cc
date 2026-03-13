#include <parser.h>

#include <cassert>
#include <sstream>

namespace rumina {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token& Parser::currentToken() const {
    if (current_ < tokens_.size()) {
        return tokens_[current_];
    }
    static Token eof(TokenType::Eof, 0, 0);
    return eof;
}

Token Parser::advance() {
    if (current_ < tokens_.size()) {
        return tokens_[current_++];
    }
    return Token(TokenType::Eof, 0, 0);
}

bool Parser::match(TokenType type) {
    if (currentToken().type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::peek(TokenType type) const {
    return currentToken().type == type;
}

bool Parser::check(TokenType type) const {
    if (current_ < tokens_.size()) {
        return tokens_[current_].type == type;
    }
    return false;
}

Token Parser::expect(TokenType type, const std::string& error_msg) {
    if (currentToken().type == type) {
        return advance();
    }
    throw std::runtime_error(error_msg + ", found " + tokenTypeToString(currentToken().type));
}

Token Parser::previous() const {
    if (current_ > 0) {
        return tokens_[current_ - 1];
    }
    return Token(TokenType::Eof, 0, 0);
}

std::unique_ptr<Expr> Parser::decimalToRational(const std::string& decimal_str) {
    size_t dot_pos = decimal_str.find('.');
    if (dot_pos == std::string::npos) {
        throw std::runtime_error("Invalid decimal format: " + decimal_str);
    }

    std::string integer_part = decimal_str.substr(0, dot_pos);
    std::string fractional_part = decimal_str.substr(dot_pos + 1);

    size_t num_decimal_places = fractional_part.length();
    if (num_decimal_places > 18) {
        throw std::runtime_error("Too many decimal places (max 18): " + decimal_str);
    }

    int64_t denominator = 1;
    for (size_t i = 0; i < num_decimal_places; ++i) {
        denominator *= 10;
    }

    std::string numerator_str = integer_part + fractional_part;
    int64_t numerator = std::stoll(numerator_str);

    return std::make_unique<BinaryExpr>(
        std::make_unique<IntExpr>(numerator),
        BinOp::Div,
        std::make_unique<IntExpr>(denominator)
    );
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (currentToken().type != TokenType::Eof) {
        statements.push_back(parseStatement());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    switch (currentToken().type) {
        case TokenType::Var:
            return parseVarDeclWithType(std::nullopt, false);
        case TokenType::Let:
            return parseVarDeclWithType(std::nullopt, true);
        case TokenType::BigInt:
            return parseVarDeclWithType(DeclaredType::BigInt, false);
        case TokenType::TypeInt:
            return parseVarDeclWithType(DeclaredType::Int, false);
        case TokenType::TypeFloat:
            return parseVarDeclWithType(DeclaredType::Float, false);
        case TokenType::TypeBool:
            return parseVarDeclWithType(DeclaredType::Bool, false);
        case TokenType::TypeString:
            if (current_ + 1 < tokens_.size() && 
                tokens_[current_ + 1].type == TokenType::DoubleColon) {
                auto expr = parseExpression();
                if (match(TokenType::Equal)) {
                    auto value = parseExpression();
                    match(TokenType::Semicolon);
                    
                    if (auto ident = dynamic_cast<IdentExpr*>(expr.get())) {
                        return std::make_unique<AssignStmt>(ident->name, std::move(value));
                    } else if (auto member = dynamic_cast<MemberExpr*>(expr.get())) {
                        return std::make_unique<MemberAssignStmt>(
                            std::move(member->object), member->member, std::move(value));
                    }
                    throw std::runtime_error("Invalid assignment target");
                } else {
                    match(TokenType::Semicolon);
                    return std::make_unique<ExprStmt>(std::move(expr));
                }
            } else {
                return parseVarDeclWithType(DeclaredType::String, false);
            }
        case TokenType::TypeRational:
            return parseVarDeclWithType(DeclaredType::Rational, false);
        case TokenType::TypeIrrational:
            return parseVarDeclWithType(DeclaredType::Irrational, false);
        case TokenType::TypeComplex:
            return parseVarDeclWithType(DeclaredType::Complex, false);
        case TokenType::TypeArray:
            return parseVarDeclWithType(DeclaredType::Array, false);
        case TokenType::Struct:
            return parseStructDecl();
        case TokenType::At:
            return parseDecoratedFuncDef();
        case TokenType::Func:
            return parseFuncDefWithDecorators({});
        case TokenType::Return:
            return parseReturn();
        case TokenType::If:
            return parseIf();
        case TokenType::While:
            return parseWhile();
        case TokenType::For:
            return parseFor();
        case TokenType::Loop:
            return parseLoop();
        case TokenType::Break:
            advance();
            match(TokenType::Semicolon);
            return std::make_unique<BreakStmt>();
        case TokenType::Continue:
            advance();
            match(TokenType::Semicolon);
            return std::make_unique<ContinueStmt>();
        case TokenType::Include:
            return parseInclude();
        case TokenType::LBrace:
            return parseBlock();
        case TokenType::Ident: {
            auto expr = parseExpression();
            
            if (match(TokenType::Equal)) {
                auto value = parseExpression();
                match(TokenType::Semicolon);
                
                if (auto ident = dynamic_cast<IdentExpr*>(expr.get())) {
                    return std::make_unique<AssignStmt>(ident->name, std::move(value));
                } else if (auto member = dynamic_cast<MemberExpr*>(expr.get())) {
                    return std::make_unique<MemberAssignStmt>(
                        std::move(member->object), member->member, std::move(value));
                }
                throw std::runtime_error("Invalid assignment target");
            } else {
                match(TokenType::Semicolon);
                return std::make_unique<ExprStmt>(std::move(expr));
            }
        }
        case TokenType::Semicolon:
            advance();
            return std::make_unique<EmptyStmt>();
        default:
            auto expr = parseExpression();
            match(TokenType::Semicolon);
            return std::make_unique<ExprStmt>(std::move(expr));
    }
}

std::unique_ptr<Stmt> Parser::parseVarDeclWithType(
    std::optional<DeclaredType> declared_type, bool immutable) {
    
    advance();
    
    if (currentToken().type != TokenType::Ident) {
        throw std::runtime_error("Expected identifier");
    }
    std::string name = std::get<std::string>(currentToken().value);
    advance();
    
    expect(TokenType::Equal, "Expected '='");
    auto value = parseExpression();
    match(TokenType::Semicolon);
    
    bool is_bigint = (declared_type == DeclaredType::BigInt);
    
    if (immutable) {
        return std::make_unique<LetDeclStmt>(name, is_bigint, declared_type, std::move(value));
    } else {
        return std::make_unique<VarDeclStmt>(name, is_bigint, declared_type, std::move(value));
    }
}

std::unique_ptr<Stmt> Parser::parseStructDecl() {
    advance();
    
    if (currentToken().type != TokenType::Ident) {
        throw std::runtime_error("Expected struct name");
    }
    std::string name = std::get<std::string>(currentToken().value);
    advance();
    
    auto value = parseStruct();
    
    return std::make_unique<VarDeclStmt>(name, false, std::nullopt, std::move(value));
}

std::unique_ptr<Stmt> Parser::parseDecoratedFuncDef() {
    std::vector<std::string> decorators;
    
    while (currentToken().type == TokenType::At) {
        advance();
        
        if (currentToken().type == TokenType::Ident) {
            decorators.push_back(std::get<std::string>(currentToken().value));
            advance();
        } else {
            throw std::runtime_error("Expected decorator name after @");
        }
    }
    
    if (currentToken().type != TokenType::Func) {
        throw std::runtime_error("Expected 'func' after decorator");
    }
    
    return parseFuncDefWithDecorators(decorators);
}

std::unique_ptr<Stmt> Parser::parseFuncDefWithDecorators(std::vector<std::string> decorators) {
    advance();
    
    if (currentToken().type != TokenType::Ident) {
        throw std::runtime_error("Expected function name");
    }
    std::string name = std::get<std::string>(currentToken().value);
    advance();
    
    std::vector<std::string> params;
    if (match(TokenType::LParen)) {
        if (currentToken().type != TokenType::RParen) {
            do {
                if (currentToken().type != TokenType::Ident) {
                    throw std::runtime_error("Expected parameter name");
                }
                params.push_back(std::get<std::string>(currentToken().value));
                advance();
            } while (match(TokenType::Comma));
        }
        expect(TokenType::RParen, "Expected ')' after parameters");
    }
    
    expect(TokenType::LBrace, "Expected '{' before function body");
    auto body = parseBlockStatements();
    expect(TokenType::RBrace, "Expected '}' after function body");
    
    return std::make_unique<FuncDefStmt>(name, params, std::move(body), decorators);
}

std::unique_ptr<Stmt> Parser::parseReturn() {
    advance();
    
    if (currentToken().type == TokenType::Semicolon) {
        advance();
        return std::make_unique<ReturnStmt>();
    } else {
        auto expr = parseExpression();
        match(TokenType::Semicolon);
        return std::make_unique<ReturnStmt>(std::move(expr));
    }
}

std::unique_ptr<Stmt> Parser::parseIf() {
    advance();
    
    auto condition = parseExpression();
    
    std::vector<std::unique_ptr<Stmt>> then_branch;
    if (currentToken().type == TokenType::LBrace) {
        advance();
        then_branch = parseBlockStatements();
        expect(TokenType::RBrace, "Expected '}' after then branch");
    } else {
        then_branch.push_back(parseStatement());
    }
    
    std::optional<std::vector<std::unique_ptr<Stmt>>> else_branch = std::nullopt;
    if (match(TokenType::Else)) {
        std::vector<std::unique_ptr<Stmt>> else_stmts;
        if (currentToken().type == TokenType::LBrace) {
            advance();
            else_stmts = parseBlockStatements();
            expect(TokenType::RBrace, "Expected '}' after else branch");
        } else {
            else_stmts.push_back(parseStatement());
        }
        else_branch = std::move(else_stmts);
    }
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch), 
                                     std::move(else_branch));
}

std::unique_ptr<Stmt> Parser::parseWhile() {
    advance();
    
    auto condition = parseExpression();
    
    std::vector<std::unique_ptr<Stmt>> body;
    if (currentToken().type == TokenType::LBrace) {
        advance();
        body = parseBlockStatements();
        expect(TokenType::RBrace, "Expected '}' after while body");
    } else {
        body.push_back(parseStatement());
    }
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseLoop() {
    advance();
    
    std::vector<std::unique_ptr<Stmt>> body;
    if (currentToken().type == TokenType::LBrace) {
        advance();
        body = parseBlockStatements();
        expect(TokenType::RBrace, "Expected '}' after loop body");
    } else {
        body.push_back(parseStatement());
    }
    
    return std::make_unique<LoopStmt>(std::move(body));
}

std::unique_ptr<Stmt> Parser::parseFor() {
    advance();
    
    expect(TokenType::LParen, "Expected '(' after for");
    
    std::optional<std::unique_ptr<Stmt>> init = std::nullopt;
    if (currentToken().type != TokenType::Semicolon) {
        init = parseStatement();
    }
    
    if (currentToken().type == TokenType::Semicolon) {
        advance();
    }
    
    std::optional<std::unique_ptr<Expr>> condition = std::nullopt;
    if (currentToken().type != TokenType::Semicolon) {
        condition = parseExpression();
    }
    expect(TokenType::Semicolon, "Expected ';' after condition");
    
    std::optional<std::unique_ptr<Stmt>> update = std::nullopt;
    if (currentToken().type != TokenType::RParen) {
        auto expr = parseExpression();
        
        if (currentToken().type == TokenType::Equal) {
            advance();
            auto value = parseExpression();
            
            if (auto ident = dynamic_cast<IdentExpr*>(expr.get())) {
                update = std::make_unique<AssignStmt>(ident->name, std::move(value));
            } else {
                throw std::runtime_error("Invalid assignment target in for update");
            }
        } else {
            update = std::make_unique<ExprStmt>(std::move(expr));
        }
    }
    expect(TokenType::RParen, "Expected ')' after for clauses");
    
    std::vector<std::unique_ptr<Stmt>> body;
    if (currentToken().type == TokenType::LBrace) {
        advance();
        body = parseBlockStatements();
        expect(TokenType::RBrace, "Expected '}' after for body");
    } else {
        body.push_back(parseStatement());
    }
    
    return std::make_unique<ForStmt>(std::move(init), std::move(condition), 
                                      std::move(update), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseInclude() {
    advance();
    
    std::string path;
    if (currentToken().type == TokenType::String) {
        path = std::get<std::string>(currentToken().value);
    } else if (currentToken().type == TokenType::Ident) {
        path = std::get<std::string>(currentToken().value);
    } else {
        throw std::runtime_error("Expected string or identifier");
    }
    advance();
    
    match(TokenType::Semicolon);
    
    return std::make_unique<IncludeStmt>(path);
}

std::unique_ptr<Stmt> Parser::parseBlock() {
    expect(TokenType::LBrace, "Expected '{'");
    auto statements = parseBlockStatements();
    expect(TokenType::RBrace, "Expected '}'");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::vector<std::unique_ptr<Stmt>> Parser::parseBlockStatements() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (currentToken().type != TokenType::RBrace && 
           currentToken().type != TokenType::Eof) {
        statements.push_back(parseStatement());
    }
    return statements;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parsePipeline();
}

std::unique_ptr<Expr> Parser::parsePipeline() {
    auto left = parseOr();
    
    while (match(TokenType::PipeForward)) {
        auto right = parseOr();
        left = transformPipeline(std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::transformPipeline(std::unique_ptr<Expr> left, 
                                                 std::unique_ptr<Expr> right) {
    if (auto call = dynamic_cast<CallExpr*>(right.get())) {
        std::vector<std::unique_ptr<Expr>> args;
        args.push_back(std::move(left));
        for (auto& arg : call->args) {
            args.push_back(std::move(arg));
        }
        return std::make_unique<CallExpr>(std::move(call->func), std::move(args));
    }
    
    if (dynamic_cast<IdentExpr*>(right.get()) || 
        dynamic_cast<NamespaceExpr*>(right.get()) ||
        dynamic_cast<MemberExpr*>(right.get())) {
        std::vector<std::unique_ptr<Expr>> args;
        args.push_back(std::move(left));
        return std::make_unique<CallExpr>(std::move(right), std::move(args));
    }
    
    throw std::runtime_error("Pipeline right-hand side must be a callable expression");
}

std::unique_ptr<Expr> Parser::parseOr() {
    auto left = parseAnd();
    
    while (match(TokenType::Or)) {
        auto right = parseAnd();
        left = std::make_unique<BinaryExpr>(
            std::move(left), BinOp::Or, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseAnd() {
    auto left = parseEquality();
    
    while (match(TokenType::And)) {
        auto right = parseEquality();
        left = std::make_unique<BinaryExpr>(
            std::move(left), BinOp::And, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto left = parseComparison();
    
    while (true) {
        BinOp op;
        if (match(TokenType::EqualEqual)) {
            op = BinOp::Equal;
        } else if (match(TokenType::BangEqual)) {
            op = BinOp::NotEqual;
        } else {
            break;
        }
        
        auto right = parseComparison();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseAddition();
    
    while (true) {
        BinOp op;
        if (match(TokenType::Greater)) {
            op = BinOp::Greater;
        } else if (match(TokenType::GreaterEqual)) {
            op = BinOp::GreaterEq;
        } else if (match(TokenType::Less)) {
            op = BinOp::Less;
        } else if (match(TokenType::LessEqual)) {
            op = BinOp::LessEq;
        } else {
            break;
        }
        
        auto right = parseAddition();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseAddition() {
    auto left = parseMultiplication();
    
    while (true) {
        BinOp op;
        if (match(TokenType::Plus)) {
            op = BinOp::Add;
        } else if (match(TokenType::Minus)) {
            op = BinOp::Sub;
        } else {
            break;
        }
        
        auto right = parseMultiplication();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplication() {
    auto left = parsePower();
    
    while (true) {
        BinOp op;
        if (match(TokenType::Star)) {
            op = BinOp::Mul;
        } else if (match(TokenType::Slash)) {
            op = BinOp::Div;
        } else if (match(TokenType::Percent)) {
            op = BinOp::Mod;
        } else {
            break;
        }
        
        auto right = parsePower();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parsePower() {
    auto left = parseUnary();
    
    if (match(TokenType::Caret)) {
        auto right = parsePower();
        left = std::make_unique<BinaryExpr>(std::move(left), BinOp::Pow, std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::Minus)) {
        auto expr = parseUnary();
        return std::make_unique<UnaryExpr>(UnaryOp::Neg, std::move(expr));
    }
    
    if (match(TokenType::Bang)) {
        auto expr = parseUnary();
        return std::make_unique<UnaryExpr>(UnaryOp::Not, std::move(expr));
    }
    
    return parsePostfix();
}

std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match(TokenType::LParen)) {
            auto args = parseArguments();
            expect(TokenType::RParen, "Expected ')' after arguments");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        } else if (match(TokenType::LBracket)) {
            auto index = parseExpression();
            expect(TokenType::RBracket, "Expected ']' after index");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else if (match(TokenType::Dot)) {
            if (currentToken().type != TokenType::Ident) {
                throw std::runtime_error("Expected member name");
            }
            std::string member = std::get<std::string>(currentToken().value);
            advance();
            expr = std::make_unique<MemberExpr>(std::move(expr), member);
        } else if (match(TokenType::Bang)) {
            expr = std::make_unique<UnaryExpr>(UnaryOp::Factorial, std::move(expr));
        } else {
            break;
        }
    }
    
    return expr;
}

std::vector<std::unique_ptr<Expr>> Parser::parseArguments() {
    std::vector<std::unique_ptr<Expr>> args;
    
    if (currentToken().type != TokenType::RParen) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }
    
    return args;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    const Token& tok = currentToken();
    
    switch (tok.type) {
        case TokenType::Int:
            advance();
            return std::make_unique<IntExpr>(std::get<int64_t>(tok.value));
            
        case TokenType::Float:
            advance();
            return std::make_unique<FloatExpr>(std::get<double>(tok.value));
            
        case TokenType::Decimal:
            advance();
            return decimalToRational(std::get<std::string>(tok.value));
            
        case TokenType::String:
            advance();
            return std::make_unique<StringExpr>(std::get<std::string>(tok.value));
            
        case TokenType::True:
            advance();
            return std::make_unique<BoolExpr>(true);
            
        case TokenType::False:
            advance();
            return std::make_unique<BoolExpr>(false);
            
        case TokenType::Null:
            advance();
            return std::make_unique<NullExpr>();
            
        case TokenType::Ident: {
            std::string name = std::get<std::string>(tok.value);
            advance();
            
            if (match(TokenType::DoubleColon)) {
                if (currentToken().type != TokenType::Ident) {
                    throw std::runtime_error("Expected identifier after '::'");
                }
                std::string member = std::get<std::string>(currentToken().value);
                advance();
                return std::make_unique<NamespaceExpr>(name, member);
            }
            return std::make_unique<IdentExpr>(name);
        }
        
        case TokenType::TypeInt:
            advance();
            if (match(TokenType::DoubleColon)) {
                if (currentToken().type != TokenType::Ident) {
                    throw std::runtime_error("Expected identifier after '::'");
                }
                std::string member = std::get<std::string>(currentToken().value);
                advance();
                return std::make_unique<NamespaceExpr>("int", member);
            }
            return std::make_unique<IdentExpr>("int");
            
        case TokenType::TypeFloat:
            advance();
            if (match(TokenType::DoubleColon)) {
                if (currentToken().type != TokenType::Ident) {
                    throw std::runtime_error("Expected identifier after '::'");
                }
                std::string member = std::get<std::string>(currentToken().value);
                advance();
                return std::make_unique<NamespaceExpr>("float", member);
            }
            return std::make_unique<IdentExpr>("float");
            
        case TokenType::TypeBool:
            advance();
            return std::make_unique<IdentExpr>("bool");
            
        case TokenType::TypeString:
            advance();
            if (match(TokenType::DoubleColon)) {
                if (currentToken().type != TokenType::Ident) {
                    throw std::runtime_error("Expected identifier after '::'");
                }
                std::string member = std::get<std::string>(currentToken().value);
                advance();
                return std::make_unique<NamespaceExpr>("string", member);
            }
            return std::make_unique<IdentExpr>("string");
            
        case TokenType::TypeRational:
            advance();
            return std::make_unique<IdentExpr>("rational");
            
        case TokenType::TypeIrrational:
            advance();
            return std::make_unique<IdentExpr>("irrational");
            
        case TokenType::TypeComplex:
            advance();
            return std::make_unique<IdentExpr>("complex");
            
        case TokenType::TypeArray:
            advance();
            return std::make_unique<IdentExpr>("array");
            
        case TokenType::LBracket:
            return parseArray();
            
        case TokenType::LBrace:
            return parseStruct();
            
        case TokenType::LParen: {
            advance();
            auto expr = parseExpression();
            expect(TokenType::RParen, "Expected ')' after expression");
            return expr;
        }
        
        case TokenType::Do:
            return parseLambda(false);
            
        case TokenType::Pipe:
            return parseLambda(true);
            
        default:
            throw std::runtime_error("Unexpected token: " + tokenTypeToString(tok.type));
    }
}

std::unique_ptr<Expr> Parser::parseArray() {
    expect(TokenType::LBracket, "Expected '['");
    std::vector<std::unique_ptr<Expr>> elements;
    
    if (currentToken().type != TokenType::RBracket) {
        do {
            elements.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }
    
    expect(TokenType::RBracket, "Expected ']' after array elements");
    return std::make_unique<ArrayExpr>(std::move(elements));
}

std::unique_ptr<Expr> Parser::parseStruct() {
    expect(TokenType::LBrace, "Expected '{'");
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields;
    
    while (currentToken().type != TokenType::RBrace && 
           currentToken().type != TokenType::Eof) {
        
        if (currentToken().type != TokenType::Ident) {
            throw std::runtime_error("Expected field name");
        }
        std::string name = std::get<std::string>(currentToken().value);
        advance();
        
        expect(TokenType::Equal, "Expected '=' after field name");
        auto value = parseExpression();
        fields.emplace_back(name, std::move(value));
        
        if (!match(TokenType::Semicolon)) {
            match(TokenType::Comma);
        }
        
        if (currentToken().type == TokenType::RBrace) {
            break;
        }
    }
    
    expect(TokenType::RBrace, "Expected '}' after struct fields");
    return std::make_unique<StructExpr>(std::move(fields));
}

std::unique_ptr<Expr> Parser::parseLambda(bool is_simple) {
    if (!is_simple) {
        expect(TokenType::Do, "Expected 'do'");
    }
    
    std::vector<std::string> params;
    if (match(TokenType::Pipe)) {
        if (currentToken().type != TokenType::Pipe) {
            do {
                if (currentToken().type != TokenType::Ident) {
                    throw std::runtime_error("Expected parameter name");
                }
                params.push_back(std::get<std::string>(currentToken().value));
                advance();
            } while (match(TokenType::Comma));
        }
        expect(TokenType::Pipe, "Expected '|' after parameters");
    }
    
    std::unique_ptr<Stmt> body;
    if (is_simple) {
        auto expr = parseExpression();
        body = std::make_unique<ReturnStmt>(std::move(expr));
    } else {
        expect(TokenType::LBrace, "Expected '{' before lambda body");
        auto stmts = parseBlockStatements();
        expect(TokenType::RBrace, "Expected '}' after lambda body");
        body = std::make_unique<BlockStmt>(std::move(stmts));
    }
    
    return std::make_unique<LambdaExpr>(params, std::move(body), is_simple);
}

} // namespace rumina
