#pragma once

#include "ast.h"
#include "vm.h"
#include "result.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace rumina {

// 符号表
struct SymbolInfo {
    std::string name;
    size_t depth;
};

class SymbolTable {
public:
    SymbolTable();
    
    void enterScope();
    void exitScope();
    void define(const std::string& name);
    const SymbolInfo* resolve(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
};

// 循环上下文
struct LoopContext {
    size_t continue_target;
    std::vector<size_t> break_patches;
};

// 字节码编译器
class Compiler {
public:
    Compiler();
    explicit Compiler(const std::string& current_dir);

    Result<ByteCode> compile(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    ByteCode bytecode_;
    SymbolTable symbols_;
    std::vector<LoopContext> loop_stack_;
    std::optional<size_t> current_line_;
    size_t lambda_counter_ = 0;
    std::unordered_set<std::string> included_files_;
    std::optional<std::string> current_dir_;
    std::unordered_map<std::string, std::string> module_namespaces_;

    void emit(OpCode op);
    size_t currentAddress() const;
    size_t emitJump(OpCode op);
    void patchJump(size_t address);

    void compileStmt(const Stmt* stmt);
    void compileExpr(const Expr* expr);
    void compileInclude(const std::string& path);
    
    std::string extractModuleName(const std::vector<std::unique_ptr<Stmt>>& statements, 
                                   const std::string& contents, 
                                   const std::string& path);
    void compileStmtWithNamespace(const Stmt* stmt, const std::string& namespace_);
};

} // namespace rumina
