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
