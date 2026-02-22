#include <interpreter.h>
#include <builtin/builtin.h>
#include <value_ops.h>
#include <lexer.h>
#include <parser.h>

#include <cmath>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace rumina {

Interpreter::Interpreter() {
    globals_ = std::make_shared<std::unordered_map<std::string, Value>>();
    builtin::register_builtins(*globals_);
    
    // LSR-010: Register imaginary unit 'i' as sqrt(-1)
    (*globals_)["i"] = Value(
        std::make_shared<Value>(Value(static_cast<int64_t>(0))),
        std::make_shared<Value>(Value(static_cast<int64_t>(1)))
    );
}

void Interpreter::setFile(const std::string& filename) {
    current_file_ = filename;
}

std::shared_ptr<std::unordered_map<std::string, Value>> Interpreter::getGlobals() {
    return globals_;
}

RuminaError Interpreter::wrapError(const std::string& message) const {
    RuminaError error = RuminaError::runtime(message);
    
    for (auto it = call_stack_.rbegin(); it != call_stack_.rend(); ++it) {
        error.addFrame({*it, current_file_, std::nullopt});
    }
    
    return error;
}

Result<std::optional<Value>> Interpreter::interpret(
    std::vector<std::unique_ptr<Stmt>> statements) {
    
    std::optional<Value> last_value;
    
    for (auto& stmt : statements) {
        try {
            if (auto expr_stmt = dynamic_cast<ExprStmt*>(stmt.get())) {
                auto result = eval_expr_impl(expr_stmt->expr.get());
                if (result.is_error()) {
                    return Err<std::optional<Value>>(result.error());
                }
                last_value = result.value();
            } else {
                execute_stmt(stmt.get());
            }
        } catch (const std::exception& e) {
            return Err<std::optional<Value>>(wrapError(e.what()).formatError());
        }
        
        if (return_value_.has_value() || break_flag_ || continue_flag_) {
            break;
        }
    }
    
    return Ok(last_value);
}

void Interpreter::setVariable(const std::string& name, const Value& value, bool immutable) {
    if (!locals_.empty()) {
        (*locals_.back())[name] = value;
        if (immutable) {
            if (!immutable_locals_.empty()) {
                immutable_locals_.back().insert(name);
            }
        }
    } else {
        (*globals_)[name] = value;
        if (immutable) {
            immutable_globals_.insert(name);
        }
    }
}

bool Interpreter::isImmutableBinding(const std::string& name) const {
    for (size_t i = 0; i < locals_.size(); ++i) {
        if (locals_[i]->find(name) != locals_[i]->end()) {
            if (i < immutable_locals_.size() && 
                immutable_locals_[i].find(name) != immutable_locals_[i].end()) {
                return true;
            }
            return false;
        }
    }
    
    if (globals_->find(name) != globals_->end()) {
        return immutable_globals_.find(name) != immutable_globals_.end();
    }
    
    return false;
}

void Interpreter::assignVariable(const std::string& name, const Value& value) {
    if (!variableExists(name)) {
        throw std::runtime_error("Variable '" + name + "' not defined");
    }
    
    if (isImmutableBinding(name)) {
        throw std::runtime_error("Cannot assign to immutable variable '" + name + "'");
    }
    
    for (auto& scope : locals_) {
        if (scope->find(name) != scope->end()) {
            (*scope)[name] = value;
            return;
        }
    }
    
    (*globals_)[name] = value;
}

Value Interpreter::getVariable(const std::string& name) const {
    for (auto it = locals_.rbegin(); it != locals_.rend(); ++it) {
        auto found = (*it)->find(name);
        if (found != (*it)->end()) {
            return found->second;
        }
    }
    
    auto found = globals_->find(name);
    if (found != globals_->end()) {
        return found->second;
    }
    
    throw std::runtime_error("Undefined variable: " + name);
}

bool Interpreter::variableExists(const std::string& name) const {
    for (const auto& scope : locals_) {
        if (scope->find(name) != scope->end()) {
            return true;
        }
    }
    return globals_->find(name) != globals_->end();
}

Value Interpreter::applyDecorator(const std::string& decorator, const Value& func) {
    if (decorator == "pure") {
        return func;
    } else if (decorator == "memoize") {
        return Value::makeMemoizedFunction(
            std::make_shared<Value>(func));
    } else {
        std::cerr << "Warning: Unknown decorator '" << decorator << "', ignoring\n";
        return func;
    }
}

Result<Value> Interpreter::eval_expr(const Expr* expr) {
    return eval_expr_impl(expr);
}

Result<Value> Interpreter::eval_expr_impl(const Expr* expr) {
    try {
        if (auto int_expr = dynamic_cast<const IntExpr*>(expr)) {
            return Ok(Value(int_expr->value));
        }
        
        else if (auto float_expr = dynamic_cast<const FloatExpr*>(expr)) {
            return Ok(Value(float_expr->value));
        }
        
        else if (auto str_expr = dynamic_cast<const StringExpr*>(expr)) {
            return Ok(Value(str_expr->value));
        }
        
        else if (auto bool_expr = dynamic_cast<const BoolExpr*>(expr)) {
            return Ok(Value(bool_expr->value));
        }
        
        else if (dynamic_cast<const NullExpr*>(expr)) {
            return Ok(Value());
        }
        
        else if (auto ident = dynamic_cast<const IdentExpr*>(expr)) {
            return Ok(getVariable(ident->name));
        }
        
        else if (auto array = dynamic_cast<const ArrayExpr*>(expr)) {
            std::vector<Value> elements;
            for (const auto& elem : array->elements) {
                auto val = eval_expr_impl(elem.get());
                if (val.is_error()) return Err<Value>(val.error());
                elements.push_back(val.value());
            }
            return Ok(Value::makeArray(
                std::make_shared<std::vector<Value>>(std::move(elements))));
        }
        
        else if (auto struct_expr = dynamic_cast<const StructExpr*>(expr)) {
            auto fields = std::make_shared<std::unordered_map<std::string, Value>>();
            for (const auto& [key, value] : struct_expr->fields) {
                auto val = eval_expr_impl(value.get());
                if (val.is_error()) return Err<Value>(val.error());
                (*fields)[key] = val.value();
            }
            return Ok(Value::makeStruct(fields));
        }
        
        else if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
            auto left = eval_expr_impl(binary->left.get());
            if (left.is_error()) return Err<Value>(left.error());
            auto right = eval_expr_impl(binary->right.get());
            if (right.is_error()) return Err<Value>(right.error());
            return eval_binary_op(left.value(), binary->op, right.value());
        }
        
        else if (auto unary = dynamic_cast<const UnaryExpr*>(expr)) {
            auto val = eval_expr_impl(unary->expr.get());
            if (val.is_error()) return Err<Value>(val.error());
            return eval_unary_op(unary->op, val.value());
        }
        
        else if (auto call = dynamic_cast<const CallExpr*>(expr)) {
            if (auto member = dynamic_cast<const MemberExpr*>(call->func.get())) {
                auto obj = eval_expr_impl(member->object.get());
                if (obj.is_error()) return Err<Value>(obj.error());
                
                if (member->member == "curried") {
                    if (!call->args.empty()) {
                        return Err<Value>("curried() does not take arguments");
                    }
                    
                    if (obj.value().getType() == Value::Type::Function ||
                        obj.value().getType() == Value::Type::Lambda) {
                        
                        size_t total_params = 0;
                        if (obj.value().getType() == Value::Type::Function) {
                            total_params = obj.value().getFunction().params.size();
                        } else {
                            total_params = obj.value().getLambda().params.size();
                        }
                        
                        return Ok(Value::makeCurriedFunction(
                            std::make_shared<Value>(obj.value()),
                            {},
                            total_params));
                    } else if (obj.value().getType() == Value::Type::NativeFunction) {
                        return Err<Value>("Cannot curry native functions");
                    } else {
                        return Err<Value>("Type " + obj.value().typeName() + 
                            " does not have method 'curried'");
                    }
                }
                
                Value method;
                if (obj.value().getType() == Value::Type::Struct ||
                    obj.value().getType() == Value::Type::Module) {
                    
                    auto struct_map = (obj.value().getType() == Value::Type::Struct) 
                        ? obj.value().getStruct() : obj.value().getModule();
                    
                    auto it = struct_map->find(member->member);
                    if (it == struct_map->end()) {
                        return Err<Value>(obj.value().typeName() + 
                            " does not have member '" + member->member + "'");
                    }
                    method = it->second;
                } else {
                    return Err<Value>("Cannot access member of " + obj.value().typeName());
                }
                
                std::vector<Value> args;
                for (const auto& arg : call->args) {
                    auto arg_val = eval_expr_impl(arg.get());
                    if (arg_val.is_error()) return Err<Value>(arg_val.error());
                    args.push_back(arg_val.value());
                }
                
                return call_method(method, obj.value(), args);
            } else {
                auto func_val = eval_expr_impl(call->func.get());
                if (func_val.is_error()) return Err<Value>(func_val.error());
                
                std::vector<Value> args;
                for (const auto& arg : call->args) {
                    auto arg_val = eval_expr_impl(arg.get());
                    if (arg_val.is_error()) return Err<Value>(arg_val.error());
                    args.push_back(arg_val.value());
                }
                
                return call_function(func_val.value(), args);
            }
        }
        
        else if (auto member = dynamic_cast<const MemberExpr*>(expr)) {
            auto obj = eval_expr_impl(member->object.get());
            if (obj.is_error()) return Err<Value>(obj.error());
            
            if (obj.value().getType() == Value::Type::Struct ||
                obj.value().getType() == Value::Type::Module) {
                
                auto struct_map = (obj.value().getType() == Value::Type::Struct) 
                    ? obj.value().getStruct() : obj.value().getModule();
                
                auto it = struct_map->find(member->member);
                if (it == struct_map->end()) {
                    return Err<Value>(obj.value().typeName() + 
                        " does not have member '" + member->member + "'");
                }
                return Ok(it->second);
            } else {
                return Err<Value>("Cannot access member of " + obj.value().typeName());
            }
        }
        
        else if (auto index = dynamic_cast<const IndexExpr*>(expr)) {
            auto obj = eval_expr_impl(index->object.get());
            if (obj.is_error()) return Err<Value>(obj.error());
            auto idx = eval_expr_impl(index->index.get());
            if (idx.is_error()) return Err<Value>(idx.error());
            
            if (obj.value().getType() == Value::Type::Array) {
                auto arr = obj.value().getArray();
                if (idx.value().getType() != Value::Type::Int) {
                    return Err<Value>("Array index must be an integer");
                }
                
                int64_t i = idx.value().getInt();
                size_t len = arr->size();
                
                if (i < 0) i = len + i;
                
                if (i < 0 || static_cast<size_t>(i) >= len) {
                    return Err<Value>("Array index out of bounds: " + std::to_string(i));
                }
                
                return Ok((*arr)[i]);
            } else if (obj.value().getType() == Value::Type::String) {
                const std::string& s = obj.value().getString();
                if (idx.value().getType() != Value::Type::Int) {
                    return Err<Value>("String index must be an integer");
                }
                
                int64_t i = idx.value().getInt();
                size_t len = s.length();
                
                if (i < 0) i = len + i;
                
                if (i < 0 || static_cast<size_t>(i) >= len) {
                    return Err<Value>("String index out of bounds: " + std::to_string(i));
                }
                
                return Ok(Value(std::string(1, s[i])));
            } else {
                return Err<Value>("Invalid indexing operation");
            }
        }
        
        else if (auto lambda = dynamic_cast<const LambdaExpr*>(expr)) {
            auto closure = locals_.empty() 
                ? globals_ 
                : std::make_shared<std::unordered_map<std::string, Value>>(*locals_.back());
            
            Value::LambdaData data;
            data.params = lambda->params;
            data.body = std::shared_ptr<Stmt>(lambda->body.get()); // 注意：这里需要克隆
            data.closure = closure;
            
            return Ok(Value::makeLambda(data));
        }
        
        else if (auto ns = dynamic_cast<const NamespaceExpr*>(expr)) {
            Value module_val = getVariable(ns->module);
            
            if (module_val.getType() == Value::Type::Module) {
                auto module = module_val.getModule();
                auto it = module->find(ns->name);
                if (it == module->end()) {
                    return Err<Value>("Module '" + ns->module + 
                        "' does not have member '" + ns->name + "'");
                }
                return Ok(it->second);
            } else {
                return Err<Value>("'" + ns->module + "' is not a module");
            }
        }
        
        return Err<Value>("Unknown expression type");
    } catch (const std::exception& e) {
        return Err<Value>(e.what());
    }
}

void Interpreter::execute_stmt(const Stmt* stmt) {
    if (auto var_decl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        auto val = eval_expr_impl(var_decl->value.get());
        if (val.is_error()) throw std::runtime_error(val.error());
        
        if (var_decl->declared_type.has_value()) {
            // Apply type conversion
            Value converted = builtin::convert_to_declared_type(val.value(), var_decl->declared_type.value());
            setVariable(var_decl->name, converted, false);
        } else if (var_decl->is_bigint) {
            // Convert to bigint
            auto result = builtin::convert_to_bigint({val.value()});
            setVariable(var_decl->name, result, false);
        } else {
            setVariable(var_decl->name, val.value(), false);
        }
    }
    
    else if (auto let_decl = dynamic_cast<const LetDeclStmt*>(stmt)) {
        auto val = eval_expr_impl(let_decl->value.get());
        if (val.is_error()) throw std::runtime_error(val.error());
        
        if (let_decl->declared_type.has_value()) {
            Value converted = builtin::convert_to_declared_type(val.value(), let_decl->declared_type.value());
            setVariable(let_decl->name, converted, true);
        } else if (let_decl->is_bigint) {
            auto result = builtin::convert_to_bigint({val.value()});
            setVariable(let_decl->name, result, true);
        } else {
            setVariable(let_decl->name, val.value(), true);
        }
    }
    
    else if (auto assign = dynamic_cast<const AssignStmt*>(stmt)) {
        auto val = eval_expr_impl(assign->value.get());
        if (val.is_error()) throw std::runtime_error(val.error());
        assignVariable(assign->name, val.value());
    }
    
    else if (auto member_assign = dynamic_cast<const MemberAssignStmt*>(stmt)) {
        auto val = eval_expr_impl(member_assign->value.get());
        if (val.is_error()) throw std::runtime_error(val.error());
        
        if (auto ident = dynamic_cast<const IdentExpr*>(member_assign->object.get())) {
            if (isImmutableBinding(ident->name)) {
                throw std::runtime_error("Cannot assign to immutable variable '" + 
                    ident->name + "'");
            }
            
            auto obj = eval_expr_impl(member_assign->object.get());
            if (obj.is_error()) throw std::runtime_error(obj.error());
            
            if (obj.value().getType() == Value::Type::Struct) {
                (*obj.value().getStruct())[member_assign->member] = val.value();
            } else if (obj.value().getType() == Value::Type::Null) {
                if (isImmutableBinding(ident->name)) {
                    throw std::runtime_error("Cannot assign to immutable variable '" + 
                        ident->name + "'");
                }
                auto new_struct = std::make_shared<std::unordered_map<std::string, Value>>();
                (*new_struct)[member_assign->member] = val.value();
                assignVariable(ident->name, Value::makeStruct(new_struct));
            } else {
                throw std::runtime_error("Cannot assign member to " + obj.value().typeName());
            }
        } else {
            auto obj = eval_expr_impl(member_assign->object.get());
            if (obj.is_error()) throw std::runtime_error(obj.error());
            if (obj.value().getType() == Value::Type::Struct) {
                (*obj.value().getStruct())[member_assign->member] = val.value();
            } else {
                throw std::runtime_error("Cannot assign member to " + obj.value().typeName());
            }
        }
    }
    
    else if (auto expr_stmt = dynamic_cast<const ExprStmt*>(stmt)) {
        auto result = eval_expr_impl(expr_stmt->expr.get());
        if (result.is_error()) throw std::runtime_error(result.error());
    }
    
    else if (auto func_def = dynamic_cast<const FuncDefStmt*>(stmt)) {
        Value::FunctionData data;
        data.name = func_def->name;
        data.params = func_def->params;
        data.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>()); // 简化
        data.decorators = func_def->decorators;
        
        Value func = Value::makeFunction(data);
        
        for (const auto& decorator : func_def->decorators) {
            func = applyDecorator(decorator, func);
        }
        
        setVariable(func_def->name, func, false);
    }
    
    else if (auto return_stmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        Value val;
        if (return_stmt->expr.has_value()) {
            auto result = eval_expr_impl(return_stmt->expr.value().get());
            if (result.is_error()) throw std::runtime_error(result.error());
            val = result.value();
        } else {
            val = Value();
        }
        return_value_ = val;
    }
    
    else if (auto if_stmt = dynamic_cast<const IfStmt*>(stmt)) {
        auto cond = eval_expr_impl(if_stmt->condition.get());
        if (cond.is_error()) throw std::runtime_error(cond.error());
        
        if (cond.value().isTruthy()) {
            for (const auto& s : if_stmt->then_branch) {
                execute_stmt(s.get());
                if (return_value_.has_value() || break_flag_ || continue_flag_) {
                    break;
                }
            }
        } else if (if_stmt->else_branch.has_value()) {
            for (const auto& s : if_stmt->else_branch.value()) {
                execute_stmt(s.get());
                if (return_value_.has_value() || break_flag_ || continue_flag_) {
                    break;
                }
            }
        }
    }
    
    else if (auto while_stmt = dynamic_cast<const WhileStmt*>(stmt)) {
        while (true) {
            auto cond = eval_expr_impl(while_stmt->condition.get());
            if (cond.is_error()) throw std::runtime_error(cond.error());
            if (!cond.value().isTruthy()) break;
            
            for (const auto& s : while_stmt->body) {
                execute_stmt(s.get());
                if (return_value_.has_value() || break_flag_) break;
                if (continue_flag_) {
                    continue_flag_ = false;
                    break;
                }
            }
            
            if (return_value_.has_value() || break_flag_) {
                break_flag_ = false;
                break;
            }
        }
    }
    
    else if (auto loop_stmt = dynamic_cast<const LoopStmt*>(stmt)) {
        while (true) {
            for (const auto& s : loop_stmt->body) {
                execute_stmt(s.get());
                if (return_value_.has_value() || break_flag_) break;
                if (continue_flag_) {
                    continue_flag_ = false;
                    break;
                }
            }
            
            if (return_value_.has_value() || break_flag_) {
                break_flag_ = false;
                break;
            }
        }
    }
    
    else if (auto for_stmt = dynamic_cast<const ForStmt*>(stmt)) {
        if (for_stmt->init.has_value()) {
            execute_stmt(for_stmt->init.value().get());
        }
        
        while (true) {
            if (for_stmt->condition.has_value()) {
                auto cond = eval_expr_impl(for_stmt->condition.value().get());
                if (cond.is_error()) throw std::runtime_error(cond.error());
                if (!cond.value().isTruthy()) break;
            }
            
            for (const auto& s : for_stmt->body) {
                execute_stmt(s.get());
                if (return_value_.has_value() || break_flag_) break;
                if (continue_flag_) {
                    continue_flag_ = false;
                    break;
                }
            }
            
            if (return_value_.has_value() || break_flag_) {
                break_flag_ = false;
                break;
            }
            
            if (for_stmt->update.has_value()) {
                execute_stmt(for_stmt->update.value().get());
            }
        }
    }
    
    else if (dynamic_cast<const BreakStmt*>(stmt)) {
        break_flag_ = true;
    }
    
    else if (dynamic_cast<const ContinueStmt*>(stmt)) {
        continue_flag_ = true;
    }
    
    else if (auto include_stmt = dynamic_cast<const IncludeStmt*>(stmt)) {
        if (include_stmt->path == "rumina:fs") {
            auto it = globals_->find("rumina:fs");
            if (it != globals_->end()) {
                (*globals_)["fs"] = it->second;
                return;
            }
            throw std::runtime_error("Built-in module 'rumina:fs' is not registered");
        }
        
        if (include_stmt->path == "rumina:path") {
            auto it = globals_->find("rumina:path");
            if (it != globals_->end()) {
                (*globals_)["path"] = it->second;
                return;
            }
            throw std::runtime_error("Built-in module 'rumina:path' is not registered");
        }
        
        if (include_stmt->path == "rumina:env") {
            auto it = globals_->find("rumina:env");
            if (it != globals_->end()) {
                (*globals_)["env"] = it->second;
                return;
            }
            throw std::runtime_error("Built-in module 'rumina:env' is not registered");
        }
        
        if (include_stmt->path == "rumina:process") {
            auto it = globals_->find("rumina:process");
            if (it != globals_->end()) {
                (*globals_)["process"] = it->second;
                return;
            }
            throw std::runtime_error("Built-in module 'rumina:process' is not registered");
        }
        
        if (include_stmt->path == "rumina:time") {
            auto it = globals_->find("rumina:time");
            if (it != globals_->end()) {
                (*globals_)["time"] = it->second;
                return;
            }
            throw std::runtime_error("Built-in module 'rumina:time' is not registered");
        }
        
        if (include_stmt->path == "rumina:stream") {
            auto it = globals_->find("rumina:stream");
            if (it != globals_->end()) {
                (*globals_)["stream"] = it->second;
                return;
            }
            throw std::runtime_error("Built-in module 'rumina:stream' is not registered");
        }
        
        if (include_stmt->path == "rumina:buffer") {
            auto it = globals_->find("rumina:buffer");
            if (it != globals_->end()) {
                (*globals_)["Buffer"] = it->second;
                return;
            }
            throw std::runtime_error("Built-in module 'rumina:buffer' is not registered");
        }
        
        if (include_stmt->path.rfind("rumina:", 0) == 0) {
            throw std::runtime_error("Unknown built-in module '" + include_stmt->path + "'");
        }
        
        std::string file_path = include_stmt->path;
        if (file_path.size() < 3 || file_path.substr(file_path.size() - 3) != ".lm") {
            file_path += ".lm";
        }
        
        std::string contents;
        std::ifstream file(file_path);
        if (!file.is_open()) {
            std::string examples_path = "examples/" + file_path;
            file.open(examples_path);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot read module '" + file_path + "'");
            }
        }
        
        contents.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
        
        std::string module_name;
        std::istringstream iss(contents);
        std::string first_line;
        std::getline(iss, first_line);
        
        if (first_line.rfind("// Module:", 0) == 0) {
            module_name = first_line.substr(10);
            size_t start = module_name.find_first_not_of(" \t");
            size_t end = module_name.find_last_not_of(" \t");
            module_name = module_name.substr(start, end - start + 1);
        } else {
            size_t last_slash = include_stmt->path.find_last_of("/\\");
            module_name = (last_slash == std::string::npos) 
                ? include_stmt->path 
                : include_stmt->path.substr(last_slash + 1);
            if (module_name.size() >= 3 && 
                module_name.substr(module_name.size() - 3) == ".lm") {
                module_name = module_name.substr(0, module_name.size() - 3);
            }
        }
        
        Lexer lexer(contents);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto statements = parser.parse();
        
        auto module_scope = std::make_shared<std::unordered_map<std::string, Value>>();
        locals_.push_back(module_scope);
        immutable_locals_.emplace_back();
        
        for (auto& stmt : statements) {
            execute_stmt(stmt.get());
        }
        
        locals_.pop_back();
        immutable_locals_.pop_back();
        
        (*globals_)[module_name] = Value::makeModule(module_scope);
    }
    
    else if (auto block = dynamic_cast<const BlockStmt*>(stmt)) {
        for (const auto& s : block->statements) {
            execute_stmt(s.get());
            if (return_value_.has_value() || break_flag_ || continue_flag_) {
                break;
            }
        }
    }
    
    else if (dynamic_cast<const EmptyStmt*>(stmt)) {
        // Do nothing
    }
    
    else {
        throw std::runtime_error("Unknown statement type");
    }
}

Result<Value> Interpreter::call_function(const Value& func, std::vector<Value> args) {
    try {
        if (func.getType() == Value::Type::MemoizedFunction) {
            auto mf = func.getMemoizedFunction();
            
            std::string cache_key;
            for (const auto& arg : args) {
                if (!cache_key.empty()) cache_key += ",";
                cache_key += arg.toString();
            }
            
            auto it = mf.cache->find(cache_key);
            if (it != mf.cache->end()) {
                return Ok(it->second);
            }
            
            auto result = call_function(*mf.original, args);
            if (result.is_error()) return result;
            
            (*mf.cache)[cache_key] = result.value();
            
            return result;
        }
        
        else if (func.getType() == Value::Type::CurriedFunction) {
            auto cf = func.getCurriedFunction();
            
            std::vector<Value> all_args = cf.collected_args;
            all_args.insert(all_args.end(), args.begin(), args.end());
            
            if (all_args.size() >= cf.total_params) {
                if (all_args.size() > cf.total_params) {
                    return Err<Value>("Too many arguments: expected " + 
                        std::to_string(cf.total_params) + ", got " + 
                        std::to_string(all_args.size()));
                }
                return call_function(*cf.original, all_args);
            } else {
                return Ok(Value::makeCurriedFunction(cf.original, all_args, cf.total_params));
            }
        }
        
        else if (func.getType() == Value::Type::Function) {
            auto f = func.getFunction();
            
            if (f.params.size() != args.size()) {
                return Err<Value>("Expected " + std::to_string(f.params.size()) + 
                    " arguments, got " + std::to_string(args.size()));
            }
            
            recursion_depth_++;
            if (recursion_depth_ > MAX_RECURSION_DEPTH) {
                recursion_depth_--;
                return Err<Value>("Maximum recursion depth exceeded (" + 
                    std::to_string(MAX_RECURSION_DEPTH) + 
                    "). Consider using memoization with @memoize decorator.");
            }
            
            call_stack_.push_back(f.name);
            
            auto local_scope = std::make_shared<std::unordered_map<std::string, Value>>();
            for (size_t i = 0; i < f.params.size(); ++i) {
                (*local_scope)[f.params[i]] = args[i];
            }
            locals_.push_back(local_scope);
            immutable_locals_.emplace_back();
            
            execute_stmt(f.body.get());
            
            locals_.pop_back();
            immutable_locals_.pop_back();
            
            call_stack_.pop_back();
            
            recursion_depth_--;
            
            Value result = return_value_.value_or(Value());
            return_value_.reset();
            
            return Ok(result);
        }
        
        else if (func.getType() == Value::Type::Lambda) {
            auto lambda = func.getLambda();
            
            if (lambda.params.size() != args.size()) {
                return Err<Value>("Expected " + std::to_string(lambda.params.size()) + 
                    " arguments, got " + std::to_string(args.size()));
            }
            
            recursion_depth_++;
            if (recursion_depth_ > MAX_RECURSION_DEPTH) {
                recursion_depth_--;
                return Err<Value>("Maximum recursion depth exceeded (" + 
                    std::to_string(MAX_RECURSION_DEPTH) + ")");
            }
            
            call_stack_.push_back("<lambda>");
            
            auto local_scope = std::make_shared<std::unordered_map<std::string, Value>>(*lambda.closure);
            for (size_t i = 0; i < lambda.params.size(); ++i) {
                (*local_scope)[lambda.params[i]] = args[i];
            }
            locals_.push_back(local_scope);
            immutable_locals_.emplace_back();
            
            execute_stmt(lambda.body.get());
            
            locals_.pop_back();
            immutable_locals_.pop_back();
            
            call_stack_.pop_back();
            
            recursion_depth_--;
            
            Value result = return_value_.value_or(Value());
            return_value_.reset();
            
            return Ok(result);
        }
        
        else if (func.getType() == Value::Type::NativeFunction) {
            auto nf = func.getNativeFunction();
            
            if (nf.name == "foreach") {
                return handle_foreach(args);
            } else if (nf.name == "map") {
                return handle_map(args);
            } else if (nf.name == "filter") {
                return handle_filter(args);
            } else if (nf.name == "reduce" || nf.name == "fold") {
                return handle_reduce(args);
            }
            
            return Ok(nf.func(args));
        }
        
        return Err<Value>("Cannot call " + func.typeName());
    } catch (const std::exception& e) {
        return Err<Value>(e.what());
    }
}

Result<Value> Interpreter::call_method(const Value& func, const Value& self_obj, 
                                       std::vector<Value> args) {
    try {
        if (func.getType() == Value::Type::Function) {
            auto f = func.getFunction();
            
            if (f.params.size() != args.size()) {
                return Err<Value>("Expected " + std::to_string(f.params.size()) + 
                    " arguments, got " + std::to_string(args.size()));
            }
            
            auto local_scope = std::make_shared<std::unordered_map<std::string, Value>>();
            (*local_scope)["self"] = self_obj;
            for (size_t i = 0; i < f.params.size(); ++i) {
                (*local_scope)[f.params[i]] = args[i];
            }
            locals_.push_back(local_scope);
            immutable_locals_.emplace_back();
            
            execute_stmt(f.body.get());
            
            locals_.pop_back();
            immutable_locals_.pop_back();
            
            Value result = return_value_.value_or(Value());
            return_value_.reset();
            
            return Ok(result);
        }
        
        else if (func.getType() == Value::Type::Lambda) {
            auto lambda = func.getLambda();
            
            if (lambda.params.size() != args.size()) {
                return Err<Value>("Expected " + std::to_string(lambda.params.size()) + 
                    " arguments, got " + std::to_string(args.size()));
            }
            
            auto local_scope = std::make_shared<std::unordered_map<std::string, Value>>(*lambda.closure);
            (*local_scope)["self"] = self_obj;
            for (size_t i = 0; i < lambda.params.size(); ++i) {
                (*local_scope)[lambda.params[i]] = args[i];
            }
            locals_.push_back(local_scope);
            immutable_locals_.emplace_back();
            
            execute_stmt(lambda.body.get());
            
            locals_.pop_back();
            immutable_locals_.pop_back();
            
            Value result = return_value_.value_or(Value());
            return_value_.reset();
            
            return Ok(result);
        }
        
        else if (func.getType() == Value::Type::NativeFunction) {
            auto nf = func.getNativeFunction();
            std::vector<Value> native_args;
            native_args.reserve(args.size() + 1);
            native_args.push_back(self_obj);
            native_args.insert(native_args.end(), args.begin(), args.end());
            return Ok(nf.func(native_args));
        }
        
        return Err<Value>("Cannot call method on " + func.typeName());
    } catch (const std::exception& e) {
        return Err<Value>(e.what());
    }
}

Result<Value> Interpreter::handle_foreach(const std::vector<Value>& args) {
    if (args.size() != 2) {
        return Err<Value>("foreach expects 2 arguments (array, function)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        return Err<Value>("foreach expects array, got " + args[0].typeName());
    }
    
    auto array = args[0].getArray();
    const Value& callback = args[1];
    
    for (size_t i = 0; i < array->size(); ++i) {
        std::vector<Value> callback_args;
        callback_args.push_back(Value(static_cast<int64_t>(i)));
        callback_args.push_back((*array)[i]);
        auto result = call_function(callback, callback_args);
        if (result.is_error()) return result;
    }
    
    return Ok(Value());
}

Result<Value> Interpreter::handle_map(const std::vector<Value>& args) {
    if (args.size() != 2) {
        return Err<Value>("map expects 2 arguments (array, function)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        return Err<Value>("map expects array, got " + args[0].typeName());
    }
    
    auto array = args[0].getArray();
    const Value& callback = args[1];
    
    std::vector<Value> result;
    result.reserve(array->size());
    
    for (const auto& elem : *array) {
        std::vector<Value> callback_args = {elem};
        auto mapped = call_function(callback, callback_args);
        if (mapped.is_error()) return mapped;
        result.push_back(mapped.value());
    }
    
    return Ok(Value::makeArray(std::make_shared<std::vector<Value>>(std::move(result))));
}

Result<Value> Interpreter::handle_filter(const std::vector<Value>& args) {
    if (args.size() != 2) {
        return Err<Value>("filter expects 2 arguments (array, function)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        return Err<Value>("filter expects array, got " + args[0].typeName());
    }
    
    auto array = args[0].getArray();
    const Value& callback = args[1];
    
    std::vector<Value> result;
    
    for (const auto& elem : *array) {
        std::vector<Value> callback_args = {elem};
        auto filter_result = call_function(callback, callback_args);
        if (filter_result.is_error()) return filter_result;
        if (filter_result.value().isTruthy()) {
            result.push_back(elem);
        }
    }
    
    return Ok(Value::makeArray(std::make_shared<std::vector<Value>>(std::move(result))));
}

Result<Value> Interpreter::handle_reduce(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        return Err<Value>("reduce expects 2 or 3 arguments (array, function, [initial])");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        return Err<Value>("reduce expects array, got " + args[0].typeName());
    }
    
    auto array = args[0].getArray();
    const Value& callback = args[1];
    
    if (array->empty()) {
        if (args.size() == 3) {
            return Ok(args[2]);
        } else {
            return Err<Value>("reduce of empty array with no initial value");
        }
    }
    
    Value accumulator;
    size_t start_index;
    
    if (args.size() == 3) {
        accumulator = args[2];
        start_index = 0;
    } else {
        accumulator = (*array)[0];
        start_index = 1;
    }
    
    for (size_t i = start_index; i < array->size(); ++i) {
        std::vector<Value> callback_args = {accumulator, (*array)[i]};
        auto result = call_function(callback, callback_args);
        if (result.is_error()) return result;
        accumulator = result.value();
    }
    
    return Ok(accumulator);
}

Result<Value> Interpreter::eval_binary_op(const Value& left, BinOp op, const Value& right) {
    // This is just a wrapper that calls value_binary_op
    // The actual implementation is in value_ops.cc
    return value_binary_op(left, op, right);
}

Result<Value> Interpreter::eval_unary_op(UnaryOp op, const Value& val) {
    // This is just a wrapper that calls value_unary_op
    // The actual implementation is in value_ops.cc
    return value_unary_op(op, val);
}

Result<Value> Interpreter::compute_power(double base, double exponent) {
    // This is just a wrapper
    return rumina::compute_power(base, exponent);
}

Result<Value> Interpreter::multiply_irrationals(const IrrationalValue& a, const IrrationalValue& b) {
    // This is just a wrapper
    return rumina::multiply_irrationals(a, b);
}

} // namespace rumina
