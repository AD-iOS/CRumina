#include <optimizer.h>

#include <algorithm>

namespace rumina {

Result<std::vector<std::unique_ptr<Stmt>>> ASTOptimizer::optimize(
    std::vector<std::unique_ptr<Stmt>> statements) {
    
    std::vector<std::unique_ptr<Stmt>> optimized;
    
    for (auto& stmt : statements) {
        auto opt_result = optimizeStmt(std::move(stmt));
        if (opt_result.has_value()) {
            optimized.push_back(std::move(opt_result.value()));
        }
    }
    
    return Ok(std::move(optimized));
}

std::optional<std::unique_ptr<Stmt>> ASTOptimizer::optimizeStmt(
    std::unique_ptr<Stmt> stmt) {
    
    if (auto expr_stmt = dynamic_cast<ExprStmt*>(stmt.get())) {
        auto opt_expr = optimizeExpr(std::move(expr_stmt->expr));
        return std::make_unique<ExprStmt>(std::move(opt_expr));
    }
    
    else if (auto var_decl = dynamic_cast<VarDeclStmt*>(stmt.get())) {
        auto opt_value = optimizeExpr(std::move(var_decl->value));
        return std::make_unique<VarDeclStmt>(
            var_decl->name, 
            var_decl->is_bigint, 
            var_decl->declared_type, 
            std::move(opt_value)
        );
    }
    
    else if (auto assign = dynamic_cast<AssignStmt*>(stmt.get())) {
        auto opt_value = optimizeExpr(std::move(assign->value));
        return std::make_unique<AssignStmt>(assign->name, std::move(opt_value));
    }
    
    else if (auto block = dynamic_cast<BlockStmt*>(stmt.get())) {
        std::vector<std::unique_ptr<Stmt>> opt_stmts;
        bool has_return = false;
        
        for (auto& s : block->statements) {
            if (has_return) {
                modified_ = true;
                continue;
            }
            
            auto opt_s = optimizeStmt(std::move(s));
            if (opt_s.has_value()) {
                if (dynamic_cast<ReturnStmt*>(opt_s.value().get())) {
                    has_return = true;
                }
                opt_stmts.push_back(std::move(opt_s.value()));
            }
        }
        
        if (opt_stmts.empty()) {
            return std::nullopt;
        }
        return std::make_unique<BlockStmt>(std::move(opt_stmts));
    }
    
    else if (auto if_stmt = dynamic_cast<IfStmt*>(stmt.get())) {
        auto opt_cond = optimizeExpr(std::move(if_stmt->condition));
        
        if (auto bool_expr = dynamic_cast<BoolExpr*>(opt_cond.get())) {
            modified_ = true;
            if (bool_expr->value) {
                std::vector<std::unique_ptr<Stmt>> opt_then;
                for (auto& s : if_stmt->then_branch) {
                    auto opt_s = optimizeStmt(std::move(s));
                    if (opt_s.has_value()) {
                        opt_then.push_back(std::move(opt_s.value()));
                    }
                }
                if (opt_then.empty()) {
                    return std::nullopt;
                }
                return std::make_unique<BlockStmt>(std::move(opt_then));
            } else {
                if (if_stmt->else_branch.has_value()) {
                    std::vector<std::unique_ptr<Stmt>> opt_else;
                    for (auto& s : if_stmt->else_branch.value()) {
                        auto opt_s = optimizeStmt(std::move(s));
                        if (opt_s.has_value()) {
                            opt_else.push_back(std::move(opt_s.value()));
                        }
                    }
                    if (opt_else.empty()) {
                        return std::nullopt;
                    }
                    return std::make_unique<BlockStmt>(std::move(opt_else));
                } else {
                    return std::nullopt;
                }
            }
        }
        
        std::vector<std::unique_ptr<Stmt>> opt_then;
        for (auto& s : if_stmt->then_branch) {
            auto opt_s = optimizeStmt(std::move(s));
            if (opt_s.has_value()) {
                opt_then.push_back(std::move(opt_s.value()));
            }
        }
        
        std::optional<std::vector<std::unique_ptr<Stmt>>> opt_else = std::nullopt;
        if (if_stmt->else_branch.has_value()) {
            std::vector<std::unique_ptr<Stmt>> else_stmts;
            for (auto& s : if_stmt->else_branch.value()) {
                auto opt_s = optimizeStmt(std::move(s));
                if (opt_s.has_value()) {
                    else_stmts.push_back(std::move(opt_s.value()));
                }
            }
            if (!else_stmts.empty()) {
                opt_else = std::move(else_stmts);
            }
        }
        
        return std::make_unique<IfStmt>(
            std::move(opt_cond), 
            std::move(opt_then), 
            std::move(opt_else)
        );
    }
    
    else if (auto while_stmt = dynamic_cast<WhileStmt*>(stmt.get())) {
        auto opt_cond = optimizeExpr(std::move(while_stmt->condition));
        
        if (auto bool_expr = dynamic_cast<BoolExpr*>(opt_cond.get())) {
            if (!bool_expr->value) {
                modified_ = true;
                return std::nullopt;
            }
        }
        
        std::vector<std::unique_ptr<Stmt>> opt_body;
        for (auto& s : while_stmt->body) {
            auto opt_s = optimizeStmt(std::move(s));
            if (opt_s.has_value()) {
                opt_body.push_back(std::move(opt_s.value()));
            }
        }
        
        return std::make_unique<WhileStmt>(std::move(opt_cond), std::move(opt_body));
    }
    
    return std::move(stmt);
}

std::unique_ptr<Expr> ASTOptimizer::optimizeExpr(std::unique_ptr<Expr> expr) {
    if (auto binary = dynamic_cast<BinaryExpr*>(expr.get())) {
        auto opt_left = optimizeExpr(std::move(binary->left));
        auto opt_right = optimizeExpr(std::move(binary->right));
        
        auto left_int = dynamic_cast<IntExpr*>(opt_left.get());
        auto right_int = dynamic_cast<IntExpr*>(opt_right.get());
        
        if (left_int && right_int) {
            int64_t a = left_int->value;
            int64_t b = right_int->value;
            
            switch (binary->op) {
                case BinOp::Add:
                    modified_ = true;
                    return std::make_unique<IntExpr>(a + b);
                case BinOp::Sub:
                    modified_ = true;
                    return std::make_unique<IntExpr>(a - b);
                case BinOp::Mul:
                    modified_ = true;
                    return std::make_unique<IntExpr>(a * b);
                case BinOp::Mod:
                    if (b != 0) {
                        modified_ = true;
                        return std::make_unique<IntExpr>(a % b);
                    }
                    break;
                case BinOp::Equal:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a == b);
                case BinOp::NotEqual:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a != b);
                case BinOp::Greater:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a > b);
                case BinOp::GreaterEq:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a >= b);
                case BinOp::Less:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a < b);
                case BinOp::LessEq:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a <= b);
                default:
                    break;
            }
        }
        
        auto left_float = dynamic_cast<FloatExpr*>(opt_left.get());
        auto right_float = dynamic_cast<FloatExpr*>(opt_right.get());
        
        if (left_float && right_float) {
            double a = left_float->value;
            double b = right_float->value;
            
            switch (binary->op) {
                case BinOp::Add:
                    modified_ = true;
                    return std::make_unique<FloatExpr>(a + b);
                case BinOp::Sub:
                    modified_ = true;
                    return std::make_unique<FloatExpr>(a - b);
                case BinOp::Mul:
                    modified_ = true;
                    return std::make_unique<FloatExpr>(a * b);
                case BinOp::Div:
                    if (b != 0.0) {
                        modified_ = true;
                        return std::make_unique<FloatExpr>(a / b);
                    }
                    break;
                default:
                    break;
            }
        }
        
        auto left_bool = dynamic_cast<BoolExpr*>(opt_left.get());
        auto right_bool = dynamic_cast<BoolExpr*>(opt_right.get());
        
        if (left_bool && right_bool) {
            bool a = left_bool->value;
            bool b = right_bool->value;
            
            switch (binary->op) {
                case BinOp::And:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a && b);
                case BinOp::Or:
                    modified_ = true;
                    return std::make_unique<BoolExpr>(a || b);
                default:
                    break;
            }
        }
        
        if (binary->op == BinOp::Mul) {
            if (right_int && right_int->value == 0) {
                modified_ = true;
                return std::make_unique<IntExpr>(0);
            }
            if (left_int && left_int->value == 0) {
                modified_ = true;
                return std::make_unique<IntExpr>(0);
            }
        }
        
        if (binary->op == BinOp::Mul) {
            if (right_int && right_int->value == 1) {
                modified_ = true;
                return std::move(opt_left);
            }
            if (left_int && left_int->value == 1) {
                modified_ = true;
                return std::move(opt_right);
            }
        }
        
        if (binary->op == BinOp::Add) {
            if (right_int && right_int->value == 0) {
                modified_ = true;
                return std::move(opt_left);
            }
            if (left_int && left_int->value == 0) {
                modified_ = true;
                return std::move(opt_right);
            }
        }
        
        return std::make_unique<BinaryExpr>(
            std::move(opt_left), binary->op, std::move(opt_right)
        );
    }
    
    else if (auto unary = dynamic_cast<UnaryExpr*>(expr.get())) {
        auto opt_inner = optimizeExpr(std::move(unary->expr));
        
        if (unary->op == UnaryOp::Neg) {
            if (auto int_expr = dynamic_cast<IntExpr*>(opt_inner.get())) {
                modified_ = true;
                return std::make_unique<IntExpr>(-int_expr->value);
            }
            if (auto float_expr = dynamic_cast<FloatExpr*>(opt_inner.get())) {
                modified_ = true;
                return std::make_unique<FloatExpr>(-float_expr->value);
            }
        }
        
        if (unary->op == UnaryOp::Not) {
            if (auto bool_expr = dynamic_cast<BoolExpr*>(opt_inner.get())) {
                modified_ = true;
                return std::make_unique<BoolExpr>(!bool_expr->value);
            }
        }
        
        return std::make_unique<UnaryExpr>(unary->op, std::move(opt_inner));
    }
    
    return std::move(expr);
}

} // namespace rumina
