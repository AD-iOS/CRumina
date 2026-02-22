#include <builtin/cas.h>
#include <value_ops.h>
#include <interpreter.h>
#include <ast.h>

#include <cmath>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <stack>
#include <functional>
#include <memory>
#include <cstdlib>
#include <cstring>

namespace rumina {
namespace builtin {
namespace cas {

// 全局CAS存储
static std::mutex cas_storage_mutex;
static std::unordered_map<std::string, std::string> cas_storage;

// 表达式节点类型
enum class ExprNodeType {
    NUMBER,
    VARIABLE,
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    SIN,
    COS,
    TAN,
    EXP,
    LN,
    SQRT,
    FUNCTION
};

// 表达式节点
struct ExprNode {
    ExprNodeType type;
    double number_value;
    std::string var_name;
    std::string func_name;
    std::shared_ptr<ExprNode> left;
    std::shared_ptr<ExprNode> right;
    std::vector<std::shared_ptr<ExprNode>> args;
    
    ExprNode(double val) : type(ExprNodeType::NUMBER), number_value(val) {}
    ExprNode(const std::string& var) : type(ExprNodeType::VARIABLE), var_name(var) {}
    ExprNode(ExprNodeType t, std::shared_ptr<ExprNode> l, std::shared_ptr<ExprNode> r) 
        : type(t), left(l), right(r) {}
    ExprNode(const std::string& name, std::vector<std::shared_ptr<ExprNode>> a) 
        : type(ExprNodeType::FUNCTION), func_name(name), args(a) {}
    explicit ExprNode(ExprNodeType t) : type(t) {}
};

// 辅助函数：将AST节点转换为字符串
static std::string expr_node_to_string(std::shared_ptr<ExprNode> node) {
    if (!node) return "";
    
    switch (node->type) {
        case ExprNodeType::NUMBER: {
            std::ostringstream oss;
            oss << node->number_value;
            return oss.str();
        }
        case ExprNodeType::VARIABLE:
            return node->var_name;
        case ExprNodeType::ADD:
            return "(" + expr_node_to_string(node->left) + "+" + expr_node_to_string(node->right) + ")";
        case ExprNodeType::SUB:
            return "(" + expr_node_to_string(node->left) + "-" + expr_node_to_string(node->right) + ")";
        case ExprNodeType::MUL:
            return expr_node_to_string(node->left) + "*" + expr_node_to_string(node->right);
        case ExprNodeType::DIV:
            return expr_node_to_string(node->left) + "/" + expr_node_to_string(node->right);
        case ExprNodeType::POW:
            return expr_node_to_string(node->left) + "^" + expr_node_to_string(node->right);
        case ExprNodeType::SIN:
            return "sin(" + expr_node_to_string(node->left) + ")";
        case ExprNodeType::COS:
            return "cos(" + expr_node_to_string(node->left) + ")";
        case ExprNodeType::TAN:
            return "tan(" + expr_node_to_string(node->left) + ")";
        case ExprNodeType::EXP:
            return "exp(" + expr_node_to_string(node->left) + ")";
        case ExprNodeType::LN:
            return "ln(" + expr_node_to_string(node->left) + ")";
        case ExprNodeType::SQRT:
            return "sqrt(" + expr_node_to_string(node->left) + ")";
        case ExprNodeType::FUNCTION: {
            std::string result = node->func_name + "(";
            for (size_t i = 0; i < node->args.size(); ++i) {
                if (i > 0) result += ",";
                result += expr_node_to_string(node->args[i]);
            }
            result += ")";
            return result;
        }
    }
    return "";
}

// 辅助函数：将表达式转换为字符串
static std::string expr_to_string(const Expr* expr, const std::string& var) {
    if (!expr) return "";
    
    if (auto int_expr = dynamic_cast<const IntExpr*>(expr)) {
        return std::to_string(int_expr->value);
    }
    else if (auto float_expr = dynamic_cast<const FloatExpr*>(expr)) {
        return std::to_string(float_expr->value);
    }
    else if (auto str_expr = dynamic_cast<const StringExpr*>(expr)) {
        return "\"" + str_expr->value + "\"";
    }
    else if (auto bool_expr = dynamic_cast<const BoolExpr*>(expr)) {
        return bool_expr->value ? "true" : "false";
    }
    else if (dynamic_cast<const NullExpr*>(expr)) {
        return "null";
    }
    else if (auto ident = dynamic_cast<const IdentExpr*>(expr)) {
        return ident->name;
    }
    else if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
        std::string left_str = expr_to_string(binary->left.get(), var);
        std::string right_str = expr_to_string(binary->right.get(), var);
        
        switch (binary->op) {
            case BinOp::Add: return "(" + left_str + "+" + right_str + ")";
            case BinOp::Sub: return "(" + left_str + "-" + right_str + ")";
            case BinOp::Mul: return left_str + "*" + right_str;
            case BinOp::Div: return left_str + "/" + right_str;
            case BinOp::Mod: return left_str + "%" + right_str;
            case BinOp::Pow: return left_str + "^" + right_str;
            case BinOp::Equal: return "(" + left_str + "==" + right_str + ")";
            case BinOp::NotEqual: return "(" + left_str + "!=" + right_str + ")";
            case BinOp::Greater: return "(" + left_str + ">" + right_str + ")";
            case BinOp::GreaterEq: return "(" + left_str + ">=" + right_str + ")";
            case BinOp::Less: return "(" + left_str + "<" + right_str + ")";
            case BinOp::LessEq: return "(" + left_str + "<=" + right_str + ")";
            case BinOp::And: return "(" + left_str + "&&" + right_str + ")";
            case BinOp::Or: return "(" + left_str + "||" + right_str + ")";
            default: return "(" + left_str + " " + binOpToString(binary->op) + " " + right_str + ")";
        }
    }
    else if (auto unary = dynamic_cast<const UnaryExpr*>(expr)) {
        std::string inner_str = expr_to_string(unary->expr.get(), var);
        switch (unary->op) {
            case UnaryOp::Neg: return "(-" + inner_str + ")";
            case UnaryOp::Not: return "(!" + inner_str + ")";
            case UnaryOp::Factorial: return inner_str + "!";
        }
    }
    else if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        std::string func_str = expr_to_string(call->func.get(), var);
        std::string args_str;
        for (size_t i = 0; i < call->args.size(); ++i) {
            if (i > 0) args_str += ",";
            args_str += expr_to_string(call->args[i].get(), var);
        }
        return func_str + "(" + args_str + ")";
    }
    else if (auto member = dynamic_cast<const MemberExpr*>(expr)) {
        return expr_to_string(member->object.get(), var) + "." + member->member;
    }
    else if (auto index = dynamic_cast<const IndexExpr*>(expr)) {
        return expr_to_string(index->object.get(), var) + "[" + expr_to_string(index->index.get(), var) + "]";
    }
    else if (auto array = dynamic_cast<const ArrayExpr*>(expr)) {
        std::string result = "[";
        for (size_t i = 0; i < array->elements.size(); ++i) {
            if (i > 0) result += ",";
            result += expr_to_string(array->elements[i].get(), var);
        }
        result += "]";
        return result;
    }
    else if (auto struct_expr = dynamic_cast<const StructExpr*>(expr)) {
        std::string result = "{";
        for (size_t i = 0; i < struct_expr->fields.size(); ++i) {
            if (i > 0) result += ",";
            result += struct_expr->fields[i].first + "=" + 
                     expr_to_string(struct_expr->fields[i].second.get(), var);
        }
        result += "}";
        return result;
    }
    else if (auto lambda = dynamic_cast<const LambdaExpr*>(expr)) {
        std::string result = "|";
        for (size_t i = 0; i < lambda->params.size(); ++i) {
            if (i > 0) result += ",";
            result += lambda->params[i];
        }
        result += "| ";
        
        if (auto return_stmt = dynamic_cast<const ReturnStmt*>(lambda->body.get())) {
            if (return_stmt->expr.has_value()) {
                result += expr_to_string(return_stmt->expr.value().get(), var);
            }
        } else if (auto block = dynamic_cast<const BlockStmt*>(lambda->body.get())) {
            result += "{ ... }";
        }
        return result;
    }
    else if (auto ns = dynamic_cast<const NamespaceExpr*>(expr)) {
        return ns->module + "::" + ns->name;
    }
    
    return "";
}

// 辅助函数：将语句转换为字符串
static std::string stmt_to_string(const Stmt* stmt, const std::string& var) {
    if (!stmt) return "";
    
    if (auto expr_stmt = dynamic_cast<const ExprStmt*>(stmt)) {
        return expr_to_string(expr_stmt->expr.get(), var);
    }
    else if (auto return_stmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        if (return_stmt->expr.has_value()) {
            return expr_to_string(return_stmt->expr.value().get(), var);
        }
        return "";
    }
    else if (auto block = dynamic_cast<const BlockStmt*>(stmt)) {
        for (const auto& s : block->statements) {
            std::string result = stmt_to_string(s.get(), var);
            if (!result.empty()) return result;
        }
    }
    else if (auto var_decl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        return var_decl->name + "=" + expr_to_string(var_decl->value.get(), var);
    }
    else if (auto assign = dynamic_cast<const AssignStmt*>(stmt)) {
        return assign->name + "=" + expr_to_string(assign->value.get(), var);
    }
    
    return "";
}

// 辅助函数：将函数转换为表达式字符串
static std::string function_to_expr_string(const Value& value) {
    if (value.getType() == Value::Type::String) {
        return value.getString();
    }
    else if (value.getType() == Value::Type::Lambda) {
        auto lambda = value.getLambda();
        if (lambda.params.size() != 1) {
            throw std::runtime_error("Function must have exactly one parameter for calculus operations");
        }
        
        // 从lambda体中提取表达式
        std::string result = stmt_to_string(lambda.body.get(), lambda.params[0]);
        if (result.empty()) {
            throw std::runtime_error("Could not extract expression from lambda body");
        }
        return result;
    }
    else if (value.getType() == Value::Type::Function) {
        auto func = value.getFunction();
        if (func.params.size() != 1) {
            throw std::runtime_error("Function must have exactly one parameter for calculus operations");
        }
        
        // 从函数体中提取表达式
        std::string result = stmt_to_string(func.body.get(), func.params[0]);
        if (result.empty()) {
            throw std::runtime_error("Could not extract expression from function body");
        }
        return result;
    }
    
    throw std::runtime_error("Cannot convert " + value.typeName() + " to expression string");
}

// 解析表达式为AST
static std::shared_ptr<ExprNode> parse_expression(const std::string& expr, size_t& pos) {
    // 跳过空格
    while (pos < expr.length() && std::isspace(expr[pos])) pos++;
    
    if (pos >= expr.length()) {
        throw std::runtime_error("Unexpected end of expression");
    }
    
    // 解析数字
    if (std::isdigit(expr[pos]) || expr[pos] == '.') {
        size_t start = pos;
        while (pos < expr.length() && (std::isdigit(expr[pos]) || expr[pos] == '.')) pos++;
        std::string num_str = expr.substr(start, pos - start);
        char* endptr;
        double val = std::strtod(num_str.c_str(), &endptr);
        if (endptr != num_str.c_str() + num_str.length()) {
            throw std::runtime_error("Invalid number format: " + num_str);
        }
        return std::make_shared<ExprNode>(val);
    }
    
    // 解析变量或函数
    if (std::isalpha(expr[pos]) || expr[pos] == '_') {
        size_t start = pos;
        while (pos < expr.length() && (std::isalnum(expr[pos]) || expr[pos] == '_')) pos++;
        std::string name = expr.substr(start, pos - start);
        
        // 跳过空格
        while (pos < expr.length() && std::isspace(expr[pos])) pos++;
        
        // 如果是函数调用
        if (pos < expr.length() && expr[pos] == '(') {
            pos++; // 跳过 '('
            std::vector<std::shared_ptr<ExprNode>> args;
            
            while (pos < expr.length() && expr[pos] != ')') {
                // 跳过空格
                while (pos < expr.length() && std::isspace(expr[pos])) pos++;
                
                if (expr[pos] == ')') break;
                
                auto arg = parse_expression(expr, pos);
                args.push_back(arg);
                
                // 跳过空格
                while (pos < expr.length() && std::isspace(expr[pos])) pos++;
                
                if (pos < expr.length() && expr[pos] == ',') {
                    pos++; // 跳过 ','
                    continue;
                }
            }
            
            if (pos >= expr.length() || expr[pos] != ')') {
                throw std::runtime_error("Missing closing parenthesis in function call");
            }
            pos++; // 跳过 ')'
            
            // 检查是否是内置函数
            if (name == "sin") return std::make_shared<ExprNode>(ExprNodeType::SIN, args[0], nullptr);
            if (name == "cos") return std::make_shared<ExprNode>(ExprNodeType::COS, args[0], nullptr);
            if (name == "tan") return std::make_shared<ExprNode>(ExprNodeType::TAN, args[0], nullptr);
            if (name == "exp") return std::make_shared<ExprNode>(ExprNodeType::EXP, args[0], nullptr);
            if (name == "ln" || name == "log") return std::make_shared<ExprNode>(ExprNodeType::LN, args[0], nullptr);
            if (name == "sqrt") return std::make_shared<ExprNode>(ExprNodeType::SQRT, args[0], nullptr);
            
            return std::make_shared<ExprNode>(name, args);
        }
        
        return std::make_shared<ExprNode>(name);
    }
    
    // 解析括号
    if (expr[pos] == '(') {
        pos++; // 跳过 '('
        auto node = parse_expression(expr, pos);
        
        // 跳过空格
        while (pos < expr.length() && std::isspace(expr[pos])) pos++;
        
        if (pos >= expr.length() || expr[pos] != ')') {
            throw std::runtime_error("Missing closing parenthesis");
        }
        pos++; // 跳过 ')'
        return node;
    }
    
    throw std::runtime_error(std::string("Unexpected character: ") + expr[pos]);
}

// 解析完整表达式（处理运算符优先级）
static std::shared_ptr<ExprNode> parse_full_expression(const std::string& expr, size_t& pos) {
    auto left = parse_expression(expr, pos);
    
    while (pos < expr.length()) {
        // 跳过空格
        while (pos < expr.length() && std::isspace(expr[pos])) pos++;
        if (pos >= expr.length()) break;
        
        char op = expr[pos];
        if (op == '+' || op == '-' || op == '*' || op == '/' || op == '^') {
            pos++; // 跳过运算符
            
            // 跳过空格
            while (pos < expr.length() && std::isspace(expr[pos])) pos++;
            
            auto right = parse_expression(expr, pos);
            
            ExprNodeType type;
            switch (op) {
                case '+': type = ExprNodeType::ADD; break;
                case '-': type = ExprNodeType::SUB; break;
                case '*': type = ExprNodeType::MUL; break;
                case '/': type = ExprNodeType::DIV; break;
                case '^': type = ExprNodeType::POW; break;
                default: throw std::runtime_error("Unknown operator");
            }
            
            left = std::make_shared<ExprNode>(type, left, right);
        } else {
            break;
        }
    }
    
    return left;
}

// 对表达式求导
static std::shared_ptr<ExprNode> differentiate_node(std::shared_ptr<ExprNode> node, const std::string& var) {
    if (!node) return nullptr;
    
    switch (node->type) {
        case ExprNodeType::NUMBER:
            return std::make_shared<ExprNode>(0.0);
            
        case ExprNodeType::VARIABLE:
            if (node->var_name == var) {
                return std::make_shared<ExprNode>(1.0);
            } else {
                return std::make_shared<ExprNode>(0.0);
            }
            
        case ExprNodeType::ADD:
            return std::make_shared<ExprNode>(ExprNodeType::ADD,
                differentiate_node(node->left, var),
                differentiate_node(node->right, var));
            
        case ExprNodeType::SUB:
            return std::make_shared<ExprNode>(ExprNodeType::SUB,
                differentiate_node(node->left, var),
                differentiate_node(node->right, var));
            
        case ExprNodeType::MUL: {
            auto f = node->left;
            auto g = node->right;
            auto f_prime = differentiate_node(f, var);
            auto g_prime = differentiate_node(g, var);
            
            auto term1 = std::make_shared<ExprNode>(ExprNodeType::MUL, f_prime, g);
            auto term2 = std::make_shared<ExprNode>(ExprNodeType::MUL, f, g_prime);
            return std::make_shared<ExprNode>(ExprNodeType::ADD, term1, term2);
        }
        
        case ExprNodeType::DIV: {
            auto f = node->left;
            auto g = node->right;
            auto f_prime = differentiate_node(f, var);
            auto g_prime = differentiate_node(g, var);
            
            auto term1 = std::make_shared<ExprNode>(ExprNodeType::MUL, f_prime, g);
            auto term2 = std::make_shared<ExprNode>(ExprNodeType::MUL, f, g_prime);
            auto numerator = std::make_shared<ExprNode>(ExprNodeType::SUB, term1, term2);
            auto denominator = std::make_shared<ExprNode>(ExprNodeType::POW, g, std::make_shared<ExprNode>(2.0));
            
            return std::make_shared<ExprNode>(ExprNodeType::DIV, numerator, denominator);
        }
        
        case ExprNodeType::POW: {
            auto base = node->left;
            auto exp = node->right;
            
            if (exp->type == ExprNodeType::NUMBER) {
                if (base->type == ExprNodeType::VARIABLE && base->var_name == var) {
                    double n = exp->number_value;
                    auto n_minus_1 = std::make_shared<ExprNode>(n - 1.0);
                    auto new_pow = std::make_shared<ExprNode>(ExprNodeType::POW, base, n_minus_1);
                    return std::make_shared<ExprNode>(ExprNodeType::MUL, std::make_shared<ExprNode>(n), new_pow);
                } else {
                    auto n_minus_1 = std::make_shared<ExprNode>(exp->number_value - 1.0);
                    auto new_pow = std::make_shared<ExprNode>(ExprNodeType::POW, base, n_minus_1);
                    auto base_prime = differentiate_node(base, var);
                    auto term1 = std::make_shared<ExprNode>(ExprNodeType::MUL, std::make_shared<ExprNode>(exp->number_value), new_pow);
                    return std::make_shared<ExprNode>(ExprNodeType::MUL, term1, base_prime);
                }
            }
            
            auto ln_f = std::make_shared<ExprNode>(ExprNodeType::LN, base, nullptr);
            auto g_prime = differentiate_node(exp, var);
            auto term1 = std::make_shared<ExprNode>(ExprNodeType::MUL, g_prime, ln_f);
            
            auto f_prime = differentiate_node(base, var);
            auto f_prime_over_f = std::make_shared<ExprNode>(ExprNodeType::DIV, f_prime, base);
            auto term2 = std::make_shared<ExprNode>(ExprNodeType::MUL, exp, f_prime_over_f);
            
            auto sum = std::make_shared<ExprNode>(ExprNodeType::ADD, term1, term2);
            return std::make_shared<ExprNode>(ExprNodeType::MUL, node, sum);
        }
        
        case ExprNodeType::SIN: {
            auto inner = node->left;
            auto inner_prime = differentiate_node(inner, var);
            auto cos_f = std::make_shared<ExprNode>(ExprNodeType::COS, inner, nullptr);
            return std::make_shared<ExprNode>(ExprNodeType::MUL, cos_f, inner_prime);
        }
        
        case ExprNodeType::COS: {
            auto inner = node->left;
            auto inner_prime = differentiate_node(inner, var);
            auto sin_f = std::make_shared<ExprNode>(ExprNodeType::SIN, inner, nullptr);
            auto neg_sin = std::make_shared<ExprNode>(ExprNodeType::MUL, std::make_shared<ExprNode>(-1.0), sin_f);
            return std::make_shared<ExprNode>(ExprNodeType::MUL, neg_sin, inner_prime);
        }
        
        case ExprNodeType::TAN: {
            auto inner = node->left;
            auto inner_prime = differentiate_node(inner, var);
            auto cos_f = std::make_shared<ExprNode>(ExprNodeType::COS, inner, nullptr);
            auto cos_f_sq = std::make_shared<ExprNode>(ExprNodeType::POW, cos_f, std::make_shared<ExprNode>(2.0));
            auto sec_sq = std::make_shared<ExprNode>(ExprNodeType::DIV, std::make_shared<ExprNode>(1.0), cos_f_sq);
            return std::make_shared<ExprNode>(ExprNodeType::MUL, sec_sq, inner_prime);
        }
        
        case ExprNodeType::EXP: {
            auto inner = node->left;
            auto inner_prime = differentiate_node(inner, var);
            return std::make_shared<ExprNode>(ExprNodeType::MUL, node, inner_prime);
        }
        
        case ExprNodeType::LN: {
            auto inner = node->left;
            auto inner_prime = differentiate_node(inner, var);
            return std::make_shared<ExprNode>(ExprNodeType::DIV, inner_prime, inner);
        }
        
        case ExprNodeType::SQRT: {
            auto inner = node->left;
            auto inner_prime = differentiate_node(inner, var);
            auto two = std::make_shared<ExprNode>(2.0);
            auto denominator = std::make_shared<ExprNode>(ExprNodeType::MUL, two, node);
            return std::make_shared<ExprNode>(ExprNodeType::DIV, inner_prime, denominator);
        }
        
        case ExprNodeType::FUNCTION: {
            std::vector<std::shared_ptr<ExprNode>> args;
            args.push_back(node);
            for (const auto& arg : node->args) {
                args.push_back(differentiate_node(arg, var));
            }
            return std::make_shared<ExprNode>("diff", args);
        }
    }
    
    return nullptr;
}

// 对表达式积分
static std::shared_ptr<ExprNode> integrate_node(std::shared_ptr<ExprNode> node, const std::string& var) {
    if (!node) return nullptr;
    
    switch (node->type) {
        case ExprNodeType::NUMBER: {
            // ∫ c dx = c*x
            return std::make_shared<ExprNode>(ExprNodeType::MUL, node, std::make_shared<ExprNode>(var));
        }
        
        case ExprNodeType::VARIABLE: {
            if (node->var_name == var) {
                // ∫ x dx = (1/2)*x^2
                auto half = std::make_shared<ExprNode>(0.5);
                auto x_sq = std::make_shared<ExprNode>(ExprNodeType::POW, 
                    std::make_shared<ExprNode>(var), 
                    std::make_shared<ExprNode>(2.0));
                return std::make_shared<ExprNode>(ExprNodeType::MUL, half, x_sq);
            } else {
                // ∫ a dx = a*x (a是常数)
                return std::make_shared<ExprNode>(ExprNodeType::MUL, node, std::make_shared<ExprNode>(var));
            }
        }
        
        case ExprNodeType::ADD: {
            return std::make_shared<ExprNode>(ExprNodeType::ADD,
                integrate_node(node->left, var),
                integrate_node(node->right, var));
        }
        
        case ExprNodeType::SUB: {
            return std::make_shared<ExprNode>(ExprNodeType::SUB,
                integrate_node(node->left, var),
                integrate_node(node->right, var));
        }
        
        case ExprNodeType::MUL: {
            if (node->left->type == ExprNodeType::NUMBER) {
                auto integral = integrate_node(node->right, var);
                return std::make_shared<ExprNode>(ExprNodeType::MUL, node->left, integral);
            } else if (node->right->type == ExprNodeType::NUMBER) {
                auto integral = integrate_node(node->left, var);
                return std::make_shared<ExprNode>(ExprNodeType::MUL, node->right, integral);
            }
            
            if (node->left->type == ExprNodeType::VARIABLE && 
                node->left->var_name == var &&
                node->right->type == ExprNodeType::NUMBER) {
                double n = node->right->number_value;
                if (n == -1.0) {
                    return std::make_shared<ExprNode>(ExprNodeType::LN, 
                        std::make_shared<ExprNode>("abs(" + var + ")"), nullptr);
                } else {
                    auto n_plus_1 = std::make_shared<ExprNode>(n + 1.0);
                    auto x_pow_n_plus_1 = std::make_shared<ExprNode>(ExprNodeType::POW,
                        std::make_shared<ExprNode>(var),
                        n_plus_1);
                    return std::make_shared<ExprNode>(ExprNodeType::DIV, x_pow_n_plus_1, n_plus_1);
                }
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::DIV: {
            if (node->left->type == ExprNodeType::NUMBER && 
                node->left->number_value == 1.0 &&
                node->right->type == ExprNodeType::VARIABLE &&
                node->right->var_name == var) {
                return std::make_shared<ExprNode>(ExprNodeType::LN, 
                    std::make_shared<ExprNode>("abs(" + var + ")"), nullptr);
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::POW: {
            if (node->left->type == ExprNodeType::VARIABLE && 
                node->left->var_name == var &&
                node->right->type == ExprNodeType::NUMBER) {
                double n = node->right->number_value;
                if (n == -1.0) {
                    return std::make_shared<ExprNode>(ExprNodeType::LN, 
                        std::make_shared<ExprNode>("abs(" + var + ")"), nullptr);
                } else {
                    auto n_plus_1 = std::make_shared<ExprNode>(n + 1.0);
                    auto x_pow_n_plus_1 = std::make_shared<ExprNode>(ExprNodeType::POW,
                        std::make_shared<ExprNode>(var),
                        n_plus_1);
                    return std::make_shared<ExprNode>(ExprNodeType::DIV, x_pow_n_plus_1, n_plus_1);
                }
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::SIN: {
            auto inner = node->left;
            
            if (inner->type == ExprNodeType::MUL) {
                if (inner->left->type == ExprNodeType::NUMBER &&
                    inner->right->type == ExprNodeType::VARIABLE &&
                    inner->right->var_name == var) {
                    double k = inner->left->number_value;
                    auto cos_kx = std::make_shared<ExprNode>(ExprNodeType::COS, 
                        std::make_shared<ExprNode>(ExprNodeType::MUL,
                            std::make_shared<ExprNode>(k),
                            std::make_shared<ExprNode>(var)), nullptr);
                    auto neg_cos = std::make_shared<ExprNode>(ExprNodeType::MUL,
                        std::make_shared<ExprNode>(-1.0), cos_kx);
                    return std::make_shared<ExprNode>(ExprNodeType::DIV, neg_cos, std::make_shared<ExprNode>(k));
                }
            } else if (inner->type == ExprNodeType::VARIABLE && inner->var_name == var) {
                auto cos_x = std::make_shared<ExprNode>(ExprNodeType::COS, 
                    std::make_shared<ExprNode>(var), nullptr);
                return std::make_shared<ExprNode>(ExprNodeType::MUL,
                    std::make_shared<ExprNode>(-1.0), cos_x);
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::COS: {
            auto inner = node->left;
            
            if (inner->type == ExprNodeType::MUL) {
                if (inner->left->type == ExprNodeType::NUMBER &&
                    inner->right->type == ExprNodeType::VARIABLE &&
                    inner->right->var_name == var) {
                    double k = inner->left->number_value;
                    auto sin_kx = std::make_shared<ExprNode>(ExprNodeType::SIN, 
                        std::make_shared<ExprNode>(ExprNodeType::MUL,
                            std::make_shared<ExprNode>(k),
                            std::make_shared<ExprNode>(var)), nullptr);
                    return std::make_shared<ExprNode>(ExprNodeType::DIV, sin_kx, std::make_shared<ExprNode>(k));
                }
            } else if (inner->type == ExprNodeType::VARIABLE && inner->var_name == var) {
                return std::make_shared<ExprNode>(ExprNodeType::SIN, 
                    std::make_shared<ExprNode>(var), nullptr);
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::TAN: {
            auto inner = node->left;
            
            if (inner->type == ExprNodeType::VARIABLE && inner->var_name == var) {
                auto cos_x = std::make_shared<ExprNode>(ExprNodeType::COS, 
                    std::make_shared<ExprNode>(var), nullptr);
                auto ln_cos = std::make_shared<ExprNode>(ExprNodeType::LN, cos_x, nullptr);
                return std::make_shared<ExprNode>(ExprNodeType::MUL,
                    std::make_shared<ExprNode>(-1.0), ln_cos);
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::EXP: {
            auto inner = node->left;
            
            if (inner->type == ExprNodeType::MUL) {
                if (inner->left->type == ExprNodeType::NUMBER &&
                    inner->right->type == ExprNodeType::VARIABLE &&
                    inner->right->var_name == var) {
                    double k = inner->left->number_value;
                    return std::make_shared<ExprNode>(ExprNodeType::DIV, node, std::make_shared<ExprNode>(k));
                }
            } else if (inner->type == ExprNodeType::VARIABLE && inner->var_name == var) {
                return node;
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::LN: {
            auto inner = node->left;
            
            if (inner->type == ExprNodeType::VARIABLE && inner->var_name == var) {
                auto x = std::make_shared<ExprNode>(var);
                auto x_ln_x = std::make_shared<ExprNode>(ExprNodeType::MUL, x, node);
                auto x_minus = std::make_shared<ExprNode>(ExprNodeType::SUB, x_ln_x, x);
                return x_minus;
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        case ExprNodeType::SQRT: {
            auto inner = node->left;
            
            if (inner->type == ExprNodeType::MUL) {
                if (inner->left->type == ExprNodeType::NUMBER &&
                    inner->right->type == ExprNodeType::VARIABLE &&
                    inner->right->var_name == var) {
                    double k = inner->left->number_value;
                    auto two_thirds = std::make_shared<ExprNode>(2.0/3.0);
                    auto pow_3_2 = std::make_shared<ExprNode>(ExprNodeType::POW,
                        std::make_shared<ExprNode>(ExprNodeType::MUL,
                            std::make_shared<ExprNode>(k),
                            std::make_shared<ExprNode>(var)),
                        std::make_shared<ExprNode>(1.5));
                    auto term = std::make_shared<ExprNode>(ExprNodeType::MUL, two_thirds, pow_3_2);
                    return std::make_shared<ExprNode>(ExprNodeType::DIV, term, std::make_shared<ExprNode>(k));
                }
            } else if (inner->type == ExprNodeType::VARIABLE && inner->var_name == var) {
                auto two_thirds = std::make_shared<ExprNode>(2.0/3.0);
                auto x_pow_3_2 = std::make_shared<ExprNode>(ExprNodeType::POW,
                    std::make_shared<ExprNode>(var),
                    std::make_shared<ExprNode>(1.5));
                return std::make_shared<ExprNode>(ExprNodeType::MUL, two_thirds, x_pow_3_2);
            }
            
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
        
        default: {
            auto integral = std::make_shared<ExprNode>(ExprNodeType::FUNCTION);
            integral->func_name = "∫";
            integral->args.push_back(node);
            integral->args.push_back(std::make_shared<ExprNode>(var));
            return integral;
        }
    }
}

// 计算表达式的数值
static double evaluate_node(std::shared_ptr<ExprNode> node, const std::unordered_map<std::string, double>& vars) {
    if (!node) return 0.0;
    
    switch (node->type) {
        case ExprNodeType::NUMBER:
            return node->number_value;
            
        case ExprNodeType::VARIABLE: {
            auto it = vars.find(node->var_name);
            if (it != vars.end()) {
                return it->second;
            }
            throw std::runtime_error("Undefined variable: " + node->var_name);
        }
        
        case ExprNodeType::ADD:
            return evaluate_node(node->left, vars) + evaluate_node(node->right, vars);
            
        case ExprNodeType::SUB:
            return evaluate_node(node->left, vars) - evaluate_node(node->right, vars);
            
        case ExprNodeType::MUL:
            return evaluate_node(node->left, vars) * evaluate_node(node->right, vars);
            
        case ExprNodeType::DIV: {
            double right_val = evaluate_node(node->right, vars);
            if (right_val == 0.0) {
                throw std::runtime_error("Division by zero");
            }
            return evaluate_node(node->left, vars) / right_val;
        }
        
        case ExprNodeType::POW:
            return std::pow(evaluate_node(node->left, vars), evaluate_node(node->right, vars));
            
        case ExprNodeType::SIN:
            return std::sin(evaluate_node(node->left, vars));
            
        case ExprNodeType::COS:
            return std::cos(evaluate_node(node->left, vars));
            
        case ExprNodeType::TAN:
            return std::tan(evaluate_node(node->left, vars));
            
        case ExprNodeType::EXP:
            return std::exp(evaluate_node(node->left, vars));
            
        case ExprNodeType::LN: {
            double val = evaluate_node(node->left, vars);
            if (val <= 0.0) {
                throw std::runtime_error("ln argument must be positive");
            }
            return std::log(val);
        }
            
        case ExprNodeType::SQRT: {
            double val = evaluate_node(node->left, vars);
            if (val < 0.0) {
                throw std::runtime_error("sqrt argument must be non-negative");
            }
            return std::sqrt(val);
        }
            
        case ExprNodeType::FUNCTION:
            throw std::runtime_error("Cannot evaluate unknown function: " + node->func_name);
    }
    
    return 0.0;
}

// 将Value转换为表达式字符串
std::string value_to_expr_string(const Value& value) {
    if (value.getType() == Value::Type::String) {
        return value.getString();
    } else if (value.getType() == Value::Type::Lambda || 
               value.getType() == Value::Type::Function) {
        return function_to_expr_string(value);
    }
    
    throw std::runtime_error("Cannot convert " + value.typeName() + " to expression string");
}

// CAS内置函数接口
Value cas_parse(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("parse expects 1 argument");
    }

    if (args[0].getType() != Value::Type::String) {
        throw std::runtime_error("parse expects string");
    }

    const std::string& s = args[0].getString();
    
    try {
        size_t pos = 0;
        auto ast = parse_full_expression(s, pos);
        
        while (pos < s.length() && std::isspace(s[pos])) pos++;
        if (pos < s.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
        
        std::string normalized = expr_node_to_string(ast);
        return Value(normalized);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Parse error: ") + e.what());
    }
}

Value cas_differentiate(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("differentiate expects 2 arguments (expr, var)");
    }

    std::string expr_str = value_to_expr_string(args[0]);

    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("differentiate expects string variable as second argument");
    }
    std::string var = args[1].getString();

    try {
        size_t pos = 0;
        auto ast = parse_full_expression(expr_str, pos);
        
        while (pos < expr_str.length() && std::isspace(expr_str[pos])) pos++;
        if (pos < expr_str.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
        
        auto derivative = differentiate_node(ast, var);
        std::string result = expr_node_to_string(derivative);
        
        return Value(result);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Differentiation error: ") + e.what());
    }
}

Value cas_solve_linear(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("solve_linear expects 2 arguments (expr, var)");
    }

    std::string expr_str = value_to_expr_string(args[0]);

    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("solve_linear expects string variable as second argument");
    }
    std::string var = args[1].getString();

    try {
        size_t eq_pos = expr_str.find('=');
        if (eq_pos != std::string::npos) {
            std::string left = expr_str.substr(0, eq_pos);
            std::string right = expr_str.substr(eq_pos + 1);
            expr_str = "(" + left + ")-(" + right + ")";
        }
        
        size_t pos = 0;
        auto ast = parse_full_expression(expr_str, pos);
        
        while (pos < expr_str.length() && std::isspace(expr_str[pos])) pos++;
        if (pos < expr_str.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
        
        std::unordered_map<std::string, double> vars;
        double solution = 0.0;
        bool found_var = false;
        
        double left_bound = -1e6;
        double right_bound = 1e6;
        double mid;
        double f_mid;
        
        for (int iter = 0; iter < 100; iter++) {
            mid = (left_bound + right_bound) / 2;
            
            vars[var] = mid;
            f_mid = evaluate_node(ast, vars);
            
            if (std::abs(f_mid) < 1e-10) {
                solution = mid;
                found_var = true;
                break;
            }
            
            vars[var] = left_bound;
            double f_left = evaluate_node(ast, vars);
            
            if (f_left * f_mid < 0) {
                right_bound = mid;
            } else {
                left_bound = mid;
            }
        }
        
        if (!found_var) {
            throw std::runtime_error("Could not find solution for equation");
        }
        
        return Value(solution);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Solve error: ") + e.what());
    }
}

Value cas_evaluate_at(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("evaluate_at expects 3 arguments (expr, var, value)");
    }

    std::string expr_str = value_to_expr_string(args[0]);

    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("evaluate_at expects string variable as second argument");
    }
    std::string var = args[1].getString();

    double value = args[2].toFloat();

    try {
        size_t pos = 0;
        auto ast = parse_full_expression(expr_str, pos);
        
        while (pos < expr_str.length() && std::isspace(expr_str[pos])) pos++;
        if (pos < expr_str.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
        
        std::unordered_map<std::string, double> vars;
        vars[var] = value;
        
        double result = evaluate_node(ast, vars);
        return Value(result);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Evaluation error: ") + e.what());
    }
}

Value cas_store(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("store expects 2 arguments (name, expr)");
    }

    if (args[0].getType() != Value::Type::String) {
        throw std::runtime_error("store expects string name");
    }
    std::string name = args[0].getString();

    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("store expects string expression");
    }
    std::string expr = args[1].getString();

    try {
        size_t pos = 0;
        auto ast = parse_full_expression(expr, pos);
        
        while (pos < expr.length() && std::isspace(expr[pos])) pos++;
        if (pos < expr.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Invalid expression: ") + e.what());
    }

    std::lock_guard<std::mutex> lock(cas_storage_mutex);
    cas_storage[name] = expr;
    
    return Value();
}

Value cas_load(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("load expects 1 argument (name)");
    }

    if (args[0].getType() != Value::Type::String) {
        throw std::runtime_error("load expects string name");
    }
    std::string name = args[0].getString();

    std::lock_guard<std::mutex> lock(cas_storage_mutex);
    auto it = cas_storage.find(name);
    if (it != cas_storage.end()) {
        return Value(it->second);
    }

    throw std::runtime_error("Expression '" + name + "' not found in storage");
}

Value cas_numerical_derivative(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("numerical_derivative expects 3 arguments (expr, var, point)");
    }

    std::string expr_str = value_to_expr_string(args[0]);

    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("numerical_derivative expects string variable as second argument");
    }
    std::string var = args[1].getString();

    double point = args[2].toFloat();

    try {
        size_t pos = 0;
        auto ast = parse_full_expression(expr_str, pos);
        
        while (pos < expr_str.length() && std::isspace(expr_str[pos])) pos++;
        if (pos < expr_str.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
        
        const double h = 1e-8;
        std::unordered_map<std::string, double> vars;
        
        vars[var] = point + h;
        double f_plus = evaluate_node(ast, vars);
        
        vars[var] = point - h;
        double f_minus = evaluate_node(ast, vars);
        
        double derivative = (f_plus - f_minus) / (2.0 * h);
        
        return Value(derivative);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Numerical differentiation error: ") + e.what());
    }
}

Value cas_integrate(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("integrate expects 2 arguments (expr, var)");
    }

    std::string expr_str = value_to_expr_string(args[0]);

    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("integrate expects string variable as second argument");
    }
    std::string var = args[1].getString();

    try {
        size_t pos = 0;
        auto ast = parse_full_expression(expr_str, pos);
        
        while (pos < expr_str.length() && std::isspace(expr_str[pos])) pos++;
        if (pos < expr_str.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
        
        auto integral = integrate_node(ast, var);
        std::string result = expr_node_to_string(integral);
        
        return Value(result);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Integration error: ") + e.what());
    }
}

Value cas_definite_integral(const std::vector<Value>& args) {
    if (args.size() != 4) {
        throw std::runtime_error("definite_integral expects 4 arguments (expr, var, lower, upper)");
    }

    std::string expr_str = value_to_expr_string(args[0]);

    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("definite_integral expects string variable as second argument");
    }
    std::string var = args[1].getString();

    double lower = args[2].toFloat();
    double upper = args[3].toFloat();

    try {
        size_t pos = 0;
        auto ast = parse_full_expression(expr_str, pos);
        
        while (pos < expr_str.length() && std::isspace(expr_str[pos])) pos++;
        if (pos < expr_str.length()) {
            throw std::runtime_error("Unexpected characters after expression");
        }
        
        auto integrate = [&](double a, double b, double eps, int max_depth) -> double {
            auto simpson = [&](double l, double r) -> double {
                double m = (l + r) / 2;
                std::unordered_map<std::string, double> vars;
                vars[var] = l;
                double fl = evaluate_node(ast, vars);
                vars[var] = m;
                double fm = evaluate_node(ast, vars);
                vars[var] = r;
                double fr = evaluate_node(ast, vars);
                return (r - l) * (fl + 4 * fm + fr) / 6;
            };
            
            std::function<double(double, double, double, int)> adaptive_simpson;
            adaptive_simpson = [&](double l, double r, double whole, int depth) -> double {
                if (depth >= max_depth) return whole;
                
                double m = (l + r) / 2;
                double left = simpson(l, m);
                double right = simpson(m, r);
                
                if (std::abs(left + right - whole) < 15 * eps) {
                    return left + right + (left + right - whole) / 15;
                }
                
                return adaptive_simpson(l, m, left, depth + 1) + 
                       adaptive_simpson(m, r, right, depth + 1);
            };
            
            double whole = simpson(a, b);
            return adaptive_simpson(a, b, whole, 0);
        };
        
        double result = integrate(lower, upper, 1e-10, 20);
        return Value(result);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Definite integral error: ") + e.what());
    }
}

} // namespace cas
} // namespace builtin
} // namespace rumina
