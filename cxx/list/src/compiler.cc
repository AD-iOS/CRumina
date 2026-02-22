#include <compiler.h>
#include <lexer.h>
#include <parser.h>

#include <fstream>
#include <filesystem>

namespace rumina {

// SymbolTable implementation

SymbolTable::SymbolTable() {
    scopes_.emplace_back(); // Global scope
}

void SymbolTable::enterScope() {
    scopes_.emplace_back();
}

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

void SymbolTable::define(const std::string& name) {
    size_t depth = scopes_.size() - 1;
    scopes_.back()[name] = SymbolInfo{name, depth};
}

const SymbolInfo* SymbolTable::resolve(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

// Compiler implementation

Compiler::Compiler() : current_dir_(std::nullopt) {}

Compiler::Compiler(const std::string& current_dir) : current_dir_(current_dir) {}

void Compiler::emit(OpCode op) {
    bytecode_.emit(std::move(op), current_line_);
}

size_t Compiler::currentAddress() const {
    return bytecode_.currentAddress();
}

size_t Compiler::emitJump(OpCode op) {
    size_t addr = currentAddress();
    emit(std::move(op));
    return addr;
}

void Compiler::patchJump(size_t address) {
    size_t target = currentAddress();
    bytecode_.patchJump(address, target);
}

Result<ByteCode> Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& statements) {
    try {
        for (const auto& stmt : statements) {
            compileStmt(stmt.get());
        }
        
        emit(OpCode(OpCodeType::Halt));
        return Ok(std::move(bytecode_));
    } catch (const std::exception& e) {
        return Err<ByteCode>(e.what());
    }
}

void Compiler::compileStmt(const Stmt* stmt) {
    if (auto expr_stmt = dynamic_cast<const ExprStmt*>(stmt)) {
        compileExpr(expr_stmt->expr.get());
    }
    
    else if (auto var_decl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        compileExpr(var_decl->value.get());
        
        if (var_decl->declared_type.has_value()) {
            emit(OpCode(OpCodeType::ConvertType, var_decl->declared_type.value()));
        } else if (var_decl->is_bigint) {
            emit(OpCode(OpCodeType::ConvertType, DeclaredType::BigInt));
        }
        
        emit(OpCode(OpCodeType::PopVar, var_decl->name));
        symbols_.define(var_decl->name);
    }
    
    else if (auto let_decl = dynamic_cast<const LetDeclStmt*>(stmt)) {
        compileExpr(let_decl->value.get());
        
        if (let_decl->declared_type.has_value()) {
            emit(OpCode(OpCodeType::ConvertType, let_decl->declared_type.value()));
        } else if (let_decl->is_bigint) {
            emit(OpCode(OpCodeType::ConvertType, DeclaredType::BigInt));
        }
        
        emit(OpCode(OpCodeType::PopVar, let_decl->name));
        emit(OpCode(OpCodeType::MarkImmutable, let_decl->name));
        symbols_.define(let_decl->name);
    }
    
    else if (auto assign = dynamic_cast<const AssignStmt*>(stmt)) {
        compileExpr(assign->value.get());
        emit(OpCode(OpCodeType::PopVar, assign->name));
    }
    
    else if (auto member_assign = dynamic_cast<const MemberAssignStmt*>(stmt)) {
        if (auto ident = dynamic_cast<const IdentExpr*>(member_assign->object.get())) {
            compileExpr(member_assign->value.get());
            emit(OpCode(OpCodeType::MemberAssignVar, 
                       std::make_pair(ident->name, member_assign->member)));
        } else {
            compileExpr(member_assign->object.get());
            compileExpr(member_assign->value.get());
            emit(OpCode(OpCodeType::MemberAssign, member_assign->member));
        }
    }
    
    else if (auto block = dynamic_cast<const BlockStmt*>(stmt)) {
        symbols_.enterScope();
        for (const auto& s : block->statements) {
            compileStmt(s.get());
        }
        symbols_.exitScope();
    }
    
    else if (auto if_stmt = dynamic_cast<const IfStmt*>(stmt)) {
        compileExpr(if_stmt->condition.get());
        
        size_t else_jump = emitJump(OpCode(OpCodeType::JumpIfFalse, size_t(0)));
        
        for (const auto& s : if_stmt->then_branch) {
            compileStmt(s.get());
        }
        
        if (if_stmt->else_branch.has_value()) {
            size_t end_jump = emitJump(OpCode(OpCodeType::Jump, size_t(0)));
            
            patchJump(else_jump);
            
            for (const auto& s : if_stmt->else_branch.value()) {
                compileStmt(s.get());
            }
            
            patchJump(end_jump);
        } else {
            patchJump(else_jump);
        }
    }
    
    else if (auto while_stmt = dynamic_cast<const WhileStmt*>(stmt)) {
        size_t loop_start = currentAddress();
        
        loop_stack_.push_back({loop_start, {}});
        
        compileExpr(while_stmt->condition.get());
        
        size_t end_jump = emitJump(OpCode(OpCodeType::JumpIfFalse, size_t(0)));
        
        for (const auto& s : while_stmt->body) {
            compileStmt(s.get());
        }
        
        emit(OpCode(OpCodeType::Jump, loop_start));
        
        patchJump(end_jump);
        
        if (!loop_stack_.empty()) {
            auto loop_ctx = loop_stack_.back();
            loop_stack_.pop_back();
            size_t break_target = currentAddress();
            for (size_t break_addr : loop_ctx.break_patches) {
                bytecode_.patchJump(break_addr, break_target);
            }
        }
    }
    
    else if (auto for_stmt = dynamic_cast<const ForStmt*>(stmt)) {
        if (for_stmt->init.has_value()) {
            compileStmt(for_stmt->init.value().get());
        }
        
        size_t condition_start = currentAddress();
        
        std::optional<size_t> end_jump = std::nullopt;
        if (for_stmt->condition.has_value()) {
            compileExpr(for_stmt->condition.value().get());
            end_jump = emitJump(OpCode(OpCodeType::JumpIfFalse, size_t(0)));
        }
        
        size_t update_placeholder = currentAddress();
        
        loop_stack_.push_back({update_placeholder, {}});
        
        size_t body_jump = emitJump(OpCode(OpCodeType::Jump, size_t(0)));
        
        size_t update_start = currentAddress();
        if (for_stmt->update.has_value()) {
            compileStmt(for_stmt->update.value().get());
        }
        emit(OpCode(OpCodeType::Jump, condition_start));
        
        patchJump(body_jump);
        
        if (!loop_stack_.empty()) {
            loop_stack_.back().continue_target = update_start;
        }
        
        for (const auto& s : for_stmt->body) {
            compileStmt(s.get());
        }
        
        emit(OpCode(OpCodeType::Jump, update_start));
        
        if (end_jump.has_value()) {
            patchJump(end_jump.value());
        }
        
        if (!loop_stack_.empty()) {
            auto loop_ctx = loop_stack_.back();
            loop_stack_.pop_back();
            size_t break_target = currentAddress();
            for (size_t break_addr : loop_ctx.break_patches) {
                bytecode_.patchJump(break_addr, break_target);
            }
        }
    }
    
    else if (auto loop_stmt = dynamic_cast<const LoopStmt*>(stmt)) {
        size_t loop_start = currentAddress();
        
        loop_stack_.push_back({loop_start, {}});
        
        for (const auto& s : loop_stmt->body) {
            compileStmt(s.get());
        }
        
        emit(OpCode(OpCodeType::Jump, loop_start));
        
        if (!loop_stack_.empty()) {
            auto loop_ctx = loop_stack_.back();
            loop_stack_.pop_back();
            size_t break_target = currentAddress();
            for (size_t break_addr : loop_ctx.break_patches) {
                bytecode_.patchJump(break_addr, break_target);
            }
        }
    }
    
    else if (auto return_stmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        if (return_stmt->expr.has_value()) {
            compileExpr(return_stmt->expr.value().get());
        } else {
            size_t idx = bytecode_.addConstant(Value());
            emit(OpCode(OpCodeType::PushConstPooled, idx));
        }
        emit(OpCode(OpCodeType::Return));
    }
    
    else if (dynamic_cast<const BreakStmt*>(stmt)) {
        size_t jump_addr = emitJump(OpCode(OpCodeType::Jump, size_t(0)));
        if (!loop_stack_.empty()) {
            loop_stack_.back().break_patches.push_back(jump_addr);
        } else {
            throw std::runtime_error("Break outside of loop");
        }
    }
    
    else if (dynamic_cast<const ContinueStmt*>(stmt)) {
        if (!loop_stack_.empty()) {
            size_t target = loop_stack_.back().continue_target;
            emit(OpCode(OpCodeType::Jump, target));
        } else {
            throw std::runtime_error("Continue outside of loop");
        }
    }
    
    else if (auto func_def = dynamic_cast<const FuncDefStmt*>(stmt)) {
        size_t skip_jump = emitJump(OpCode(OpCodeType::Jump, size_t(0)));
        
        size_t body_start = currentAddress();
        
        symbols_.enterScope();
        for (const auto& param : func_def->params) {
            symbols_.define(param);
        }
        
        for (const auto& s : func_def->body) {
            compileStmt(s.get());
        }
        
        size_t idx = bytecode_.addConstant(Value());
        emit(OpCode(OpCodeType::PushConstPooled, idx));
        emit(OpCode(OpCodeType::Return));
        
        symbols_.exitScope();
        
        size_t body_end = currentAddress();
        
        patchJump(skip_jump);
        
        FuncDefInfo info;
        info.name = func_def->name;
        info.params = func_def->params;
        info.body_start = body_start;
        info.body_end = body_end;
        info.decorators = func_def->decorators;
        
        emit(OpCode(OpCodeType::DefineFunc, info));
        
        symbols_.define(func_def->name);
    }
    
    else if (auto include_stmt = dynamic_cast<const IncludeStmt*>(stmt)) {
        compileInclude(include_stmt->path);
    }
    
    else if (dynamic_cast<const EmptyStmt*>(stmt)) {
        // Do nothing
    }
    
    else {
        throw std::runtime_error("Unimplemented statement compilation");
    }
}

void Compiler::compileInclude(const std::string& path) {
    // Handle built-in modules - 使用明确的字符串构造函数
    if (path == "rumina:fs") {
        emit(OpCode(OpCodeType::PushVar, std::string("rumina:fs")));
        emit(OpCode(OpCodeType::PopVar, std::string("fs")));
        symbols_.define("fs");
        return;
    }
    
    if (path == "rumina:path") {
        emit(OpCode(OpCodeType::PushVar, std::string("rumina:path")));
        emit(OpCode(OpCodeType::PopVar, std::string("path")));
        symbols_.define("path");
        return;
    }
    
    if (path == "rumina:env") {
        emit(OpCode(OpCodeType::PushVar, std::string("rumina:env")));
        emit(OpCode(OpCodeType::PopVar, std::string("env")));
        symbols_.define("env");
        return;
    }
    
    if (path == "rumina:process") {
        emit(OpCode(OpCodeType::PushVar, std::string("rumina:process")));
        emit(OpCode(OpCodeType::PopVar, std::string("process")));
        symbols_.define("process");
        return;
    }
    
    if (path == "rumina:time") {
        emit(OpCode(OpCodeType::PushVar, std::string("rumina:time")));
        emit(OpCode(OpCodeType::PopVar, std::string("time")));
        symbols_.define("time");
        return;
    }
    
    if (path == "rumina:stream") {
        emit(OpCode(OpCodeType::PushVar, std::string("rumina:stream")));
        emit(OpCode(OpCodeType::PopVar, std::string("stream")));
        symbols_.define("stream");
        return;
    }
    
    if (path == "rumina:buffer") {
        emit(OpCode(OpCodeType::PushVar, std::string("rumina:buffer")));
        emit(OpCode(OpCodeType::PopVar, std::string("Buffer")));
        symbols_.define("Buffer");
        return;
    }
    
    if (path.rfind("rumina:", 0) == 0) {
        throw std::runtime_error("Unknown built-in module '" + path + "'");
    }
    
    // Construct file path
    std::string file_path = path;
    if (file_path.size() < 3 || file_path.substr(file_path.size() - 3) != ".lm") {
        file_path += ".lm";
    }
    
    // Resolve relative path
    std::filesystem::path resolved_path;
    if (current_dir_.has_value()) {
        resolved_path = std::filesystem::path(current_dir_.value()) / file_path;
    } else {
        resolved_path = std::filesystem::path(file_path);
    }
    
    // Check if already included
    std::string canonical_path = std::filesystem::absolute(resolved_path).string();
    if (included_files_.find(canonical_path) != included_files_.end()) {
        return; // Already included
    }
    
    included_files_.insert(canonical_path);
    
    // Read the file
    std::ifstream file(resolved_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot read included file '" + resolved_path.string() + "'");
    }
    
    std::string contents((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    
    // Parse the included file
    Lexer lexer(contents);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto statements = parser.parse();
    
    // Extract module name
    std::string module_name = extractModuleName(statements, contents, path);
    
    // Store module namespace mapping
    module_namespaces_[module_name] = module_name;
    
    // Compile each statement with namespace prefix
    for (const auto& stmt : statements) {
        compileStmtWithNamespace(stmt.get(), module_name);
    }
}

std::string Compiler::extractModuleName(
    const std::vector<std::unique_ptr<Stmt>>& statements,
    const std::string& contents,
    const std::string& path) {
    
    for (const auto& stmt : statements) {
        if (auto var_decl = dynamic_cast<const VarDeclStmt*>(stmt.get())) {
            if (var_decl->name == "module_name") {
                if (auto str_expr = dynamic_cast<const StringExpr*>(var_decl->value.get())) {
                    return str_expr->value;
                }
            }
        }
        
        if (auto assign = dynamic_cast<const AssignStmt*>(stmt.get())) {
            if (assign->name == "module_name") {
                if (auto str_expr = dynamic_cast<const StringExpr*>(assign->value.get())) {
                    return str_expr->value;
                }
            }
        }
        
        if (auto expr_stmt = dynamic_cast<const ExprStmt*>(stmt.get())) {
            if (auto call = dynamic_cast<const CallExpr*>(expr_stmt->expr.get())) {
                if (auto ident = dynamic_cast<const IdentExpr*>(call->func.get())) {
                    if (ident->name == "define" && call->args.size() == 2) {
                        if (auto name_ident = dynamic_cast<const IdentExpr*>(call->args[0].get())) {
                            if (name_ident->name == "module_name") {
                                if (auto str_expr = dynamic_cast<const StringExpr*>(call->args[1].get())) {
                                    return str_expr->value;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    size_t last_slash = path.find_last_of("/\\");
    std::string filename = (last_slash == std::string::npos) ? path : path.substr(last_slash + 1);
    if (filename.size() >= 3 && filename.substr(filename.size() - 3) == ".lm") {
        filename = filename.substr(0, filename.size() - 3);
    }
    return filename;
}

void Compiler::compileStmtWithNamespace(const Stmt* stmt, const std::string& namespace_) {
    if (auto var_decl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        if (var_decl->name == "module_name") return;
    }
    if (auto let_decl = dynamic_cast<const LetDeclStmt*>(stmt)) {
        if (let_decl->name == "module_name") return;
    }
    if (auto assign = dynamic_cast<const AssignStmt*>(stmt)) {
        if (assign->name == "module_name") return;
    }
    
    if (auto expr_stmt = dynamic_cast<const ExprStmt*>(stmt)) {
        if (auto ident = dynamic_cast<const IdentExpr*>(expr_stmt->expr.get())) {
            if (ident->name == "define") return;
        }
    }
    
    if (auto var_decl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        std::string prefixed_name = namespace_ + "::" + var_decl->name;
        
        compileExpr(var_decl->value.get());
        
        if (var_decl->declared_type.has_value()) {
            emit(OpCode(OpCodeType::ConvertType, var_decl->declared_type.value()));
        } else if (var_decl->is_bigint) {
            emit(OpCode(OpCodeType::ConvertType, DeclaredType::BigInt));
        }
        
        emit(OpCode(OpCodeType::PopVar, prefixed_name));
        symbols_.define(prefixed_name);
        return;
    }
    
    if (auto let_decl = dynamic_cast<const LetDeclStmt*>(stmt)) {
        std::string prefixed_name = namespace_ + "::" + let_decl->name;
        
        compileExpr(let_decl->value.get());
        
        if (let_decl->declared_type.has_value()) {
            emit(OpCode(OpCodeType::ConvertType, let_decl->declared_type.value()));
        } else if (let_decl->is_bigint) {
            emit(OpCode(OpCodeType::ConvertType, DeclaredType::BigInt));
        }
        
        emit(OpCode(OpCodeType::PopVar, prefixed_name));
        emit(OpCode(OpCodeType::MarkImmutable, prefixed_name));
        symbols_.define(prefixed_name);
        return;
    }
    
    if (auto func_def = dynamic_cast<const FuncDefStmt*>(stmt)) {
        std::string prefixed_name = namespace_ + "::" + func_def->name;
        
        size_t skip_jump = emitJump(OpCode(OpCodeType::Jump, size_t(0)));
        
        size_t body_start = currentAddress();
        
        symbols_.enterScope();
        for (const auto& param : func_def->params) {
            symbols_.define(param);
        }
        
        for (const auto& s : func_def->body) {
            compileStmt(s.get());
        }
        
        size_t idx = bytecode_.addConstant(Value());
        emit(OpCode(OpCodeType::PushConstPooled, idx));
        emit(OpCode(OpCodeType::Return));
        
        symbols_.exitScope();
        
        size_t body_end = currentAddress();
        
        patchJump(skip_jump);
        
        FuncDefInfo info;
        info.name = prefixed_name;
        info.params = func_def->params;
        info.body_start = body_start;
        info.body_end = body_end;
        info.decorators = func_def->decorators;
        
        emit(OpCode(OpCodeType::DefineFunc, info));
        
        symbols_.define(prefixed_name);
        return;
    }
    
    compileStmt(stmt);
}

void Compiler::compileExpr(const Expr* expr) {
    if (auto int_expr = dynamic_cast<const IntExpr*>(expr)) {
        size_t idx = bytecode_.addConstant(Value(static_cast<int64_t>(int_expr->value)));
        emit(OpCode(OpCodeType::PushConstPooled, idx));
    }
    
    else if (auto float_expr = dynamic_cast<const FloatExpr*>(expr)) {
        size_t idx = bytecode_.addConstant(Value(float_expr->value));
        emit(OpCode(OpCodeType::PushConstPooled, idx));
    }
    
    else if (auto str_expr = dynamic_cast<const StringExpr*>(expr)) {
        size_t idx = bytecode_.addConstant(Value(str_expr->value));
        emit(OpCode(OpCodeType::PushConstPooled, idx));
    }
    
    else if (auto bool_expr = dynamic_cast<const BoolExpr*>(expr)) {
        size_t idx = bytecode_.addConstant(Value(bool_expr->value));
        emit(OpCode(OpCodeType::PushConstPooled, idx));
    }
    
    else if (dynamic_cast<const NullExpr*>(expr)) {
        size_t idx = bytecode_.addConstant(Value());
        emit(OpCode(OpCodeType::PushConstPooled, idx));
    }
    
    else if (auto ident = dynamic_cast<const IdentExpr*>(expr)) {
        emit(OpCode(OpCodeType::PushVar, ident->name));
    }
    
    else if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
        compileExpr(binary->left.get());
        compileExpr(binary->right.get());
        
        OpCodeType op_type;
        switch (binary->op) {
            case BinOp::Add: op_type = OpCodeType::Add; break;
            case BinOp::Sub: op_type = OpCodeType::Sub; break;
            case BinOp::Mul: op_type = OpCodeType::Mul; break;
            case BinOp::Div: op_type = OpCodeType::Div; break;
            case BinOp::Mod: op_type = OpCodeType::Mod; break;
            case BinOp::Pow: op_type = OpCodeType::Pow; break;
            case BinOp::Equal: op_type = OpCodeType::Eq; break;
            case BinOp::NotEqual: op_type = OpCodeType::Neq; break;
            case BinOp::Greater: op_type = OpCodeType::Gt; break;
            case BinOp::GreaterEq: op_type = OpCodeType::Gte; break;
            case BinOp::Less: op_type = OpCodeType::Lt; break;
            case BinOp::LessEq: op_type = OpCodeType::Lte; break;
            case BinOp::And: op_type = OpCodeType::And; break;
            case BinOp::Or: op_type = OpCodeType::Or; break;
            default: throw std::runtime_error("Unsupported binary operator");
        }
        
        emit(OpCode(op_type));
    }
    
    else if (auto unary = dynamic_cast<const UnaryExpr*>(expr)) {
        compileExpr(unary->expr.get());
        
        OpCodeType op_type;
        switch (unary->op) {
            case UnaryOp::Neg: op_type = OpCodeType::Neg; break;
            case UnaryOp::Not: op_type = OpCodeType::Not; break;
            case UnaryOp::Factorial: op_type = OpCodeType::Factorial; break;
            default: throw std::runtime_error("Unsupported unary operator");
        }
        
        emit(OpCode(op_type));
    }
    
    else if (auto array = dynamic_cast<const ArrayExpr*>(expr)) {
        for (const auto& elem : array->elements) {
            compileExpr(elem.get());
        }
        emit(OpCode(OpCodeType::MakeArray, array->elements.size()));
    }
    
    else if (auto struct_expr = dynamic_cast<const StructExpr*>(expr)) {
        for (const auto& [key, value] : struct_expr->fields) {
            size_t key_idx = bytecode_.addConstant(Value(key));
            emit(OpCode(OpCodeType::PushConstPooled, key_idx));
            compileExpr(value.get());
        }
        emit(OpCode(OpCodeType::MakeStruct, struct_expr->fields.size()));
    }
    
    else if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        if (auto ident = dynamic_cast<const IdentExpr*>(call->func.get())) {
            for (const auto& arg : call->args) {
                compileExpr(arg.get());
            }
            emit(OpCode(OpCodeType::CallVar, std::make_pair(ident->name, call->args.size())));
        } else if (auto ns = dynamic_cast<const NamespaceExpr*>(call->func.get())) {
            for (const auto& arg : call->args) {
                compileExpr(arg.get());
            }
            std::string prefixed_name = ns->module + "::" + ns->name;
            emit(OpCode(OpCodeType::CallVar, std::make_pair(prefixed_name, call->args.size())));
        } else if (auto member = dynamic_cast<const MemberExpr*>(call->func.get())) {
            compileExpr(member->object.get());
            emit(OpCode(OpCodeType::Dup));
            emit(OpCode(OpCodeType::Member, member->member));
            for (const auto& arg : call->args) {
                compileExpr(arg.get());
            }
            emit(OpCode(OpCodeType::CallMethod, call->args.size()));
        } else {
            compileExpr(call->func.get());
            for (const auto& arg : call->args) {
                compileExpr(arg.get());
            }
            emit(OpCode(OpCodeType::Call, call->args.size()));
        }
    }
    
    else if (auto index = dynamic_cast<const IndexExpr*>(expr)) {
        compileExpr(index->object.get());
        compileExpr(index->index.get());
        emit(OpCode(OpCodeType::Index));
    }
    
    else if (auto member = dynamic_cast<const MemberExpr*>(expr)) {
        compileExpr(member->object.get());
        emit(OpCode(OpCodeType::Member, member->member));
    }
    
    else if (auto lambda = dynamic_cast<const LambdaExpr*>(expr)) {
        std::string lambda_id = "__lambda_" + std::to_string(lambda_counter_++);
        
        size_t skip_jump = emitJump(OpCode(OpCodeType::Jump, size_t(0)));
        
        size_t body_start = currentAddress();
        
        symbols_.enterScope();
        for (const auto& param : lambda->params) {
            symbols_.define(param);
        }
        
        compileStmt(lambda->body.get());
        
        emit(OpCode(OpCodeType::Return));
        
        symbols_.exitScope();
        
        size_t body_end = currentAddress();
        
        patchJump(skip_jump);
        
        FuncDefInfo func_info;
        func_info.name = lambda_id;
        func_info.params = lambda->params;
        func_info.body_start = body_start;
        func_info.body_end = body_end;
        func_info.decorators = {};
        
        emit(OpCode(OpCodeType::DefineFunc, func_info));
        
        size_t id_idx = bytecode_.addConstant(Value(lambda_id));
        emit(OpCode(OpCodeType::PushConstPooled, id_idx));
        
        LambdaInfo lambda_info;
        lambda_info.params = lambda->params;
        lambda_info.body_start = body_start;
        lambda_info.body_end = body_end;
        
        emit(OpCode(OpCodeType::MakeLambda, lambda_info));
    }
    
    else if (auto ns = dynamic_cast<const NamespaceExpr*>(expr)) {
        std::string prefixed_name = ns->module + "::" + ns->name;
        emit(OpCode(OpCodeType::PushVar, prefixed_name));
    }
    
    else {
        throw std::runtime_error("Unimplemented expression compilation");
    }
}

} // namespace rumina
