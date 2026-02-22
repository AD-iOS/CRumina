#pragma once

#include "value.h"
#include "ast.h"
#include "error.h"
#include "result.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

namespace rumina {

// 解释器
class Interpreter {
public:
    Interpreter();
    
    void setFile(const std::string& filename);
    std::shared_ptr<std::unordered_map<std::string, Value>> getGlobals();
    
    Result<std::optional<Value>> interpret(
        std::vector<std::unique_ptr<Stmt>> statements);
    
    Result<Value> eval_expr(const Expr* expr);
    Result<Value> eval_binary_op(const Value& left, BinOp op, const Value& right);
    Result<Value> eval_unary_op(UnaryOp op, const Value& val);
    
    Result<Value> call_function(const Value& func, std::vector<Value> args);
    Result<Value> call_method(const Value& func, const Value& self_obj, 
                              std::vector<Value> args);

private:
    std::shared_ptr<std::unordered_map<std::string, Value>> globals_;
    std::unordered_set<std::string> immutable_globals_;
    std::vector<std::shared_ptr<std::unordered_map<std::string, Value>>> locals_;
    std::vector<std::unordered_set<std::string>> immutable_locals_;
    
    std::optional<Value> return_value_;
    bool break_flag_ = false;
    bool continue_flag_ = false;
    
    std::string current_file_;
    std::vector<std::string> call_stack_;
    
    size_t recursion_depth_ = 0;
    static constexpr size_t MAX_RECURSION_DEPTH = 4000;
    
    void setVariable(const std::string& name, const Value& value, bool immutable);
    bool isImmutableBinding(const std::string& name) const;
    void assignVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name) const;
    bool variableExists(const std::string& name) const;
    
    Value applyDecorator(const std::string& decorator, const Value& func);
    
    Result<Value> eval_expr_impl(const Expr* expr);
    void execute_stmt(const Stmt* stmt);
    
    Result<Value> handle_foreach(const std::vector<Value>& args);
    Result<Value> handle_map(const std::vector<Value>& args);
    Result<Value> handle_filter(const std::vector<Value>& args);
    Result<Value> handle_reduce(const std::vector<Value>& args);
    
    Result<Value> compute_power(double base, double exponent);
    Result<Value> multiply_irrationals(const IrrationalValue& a, const IrrationalValue& b);
    
    RuminaError wrapError(const std::string& message) const;
};

} // namespace rumina
