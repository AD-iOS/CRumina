#pragma once

#include "fwd.h"
#include "value.h"
#include "ast.h"
#include "result.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>

namespace rumina {

// 函数定义信息
struct FuncDefInfo {
    std::string name;
    std::vector<std::string> params;
    size_t body_start;
    size_t body_end;
    std::vector<std::string> decorators;
    
    FuncDefInfo() = default;
    FuncDefInfo(const FuncDefInfo& other) = default;
    FuncDefInfo& operator=(const FuncDefInfo& other) = default;
    FuncDefInfo(FuncDefInfo&& other) = default;
    FuncDefInfo& operator=(FuncDefInfo&& other) = default;
};

// Lambda信息
struct LambdaInfo {
    std::vector<std::string> params;
    size_t body_start;
    size_t body_end;
    
    LambdaInfo() = default;
    LambdaInfo(const LambdaInfo& other) = default;
    LambdaInfo& operator=(const LambdaInfo& other) = default;
    LambdaInfo(LambdaInfo&& other) = default;
    LambdaInfo& operator=(LambdaInfo&& other) = default;
};

// 操作码类型
enum class OpCodeType {
    PushConst, PushConstPooled, PushVar, PopVar, MarkImmutable, Dup, Pop,
    Add, Sub, Mul, Div, Mod, Pow, Neg, Factorial,
    Not, And, Or, Eq, Neq, Gt, Gte, Lt, Lte,
    Jump, JumpIfFalse, JumpIfTrue,
    CallVar, Call, CallMethod, Return,
    MakeArray, MakeStruct, Index, Member, IndexAssign, MemberAssign, MemberAssignVar,
    DefineFunc, MakeLambda,
    Break, Continue, Halt,
    ConvertType
};

// 操作码
struct OpCode {
    OpCodeType type;
    
    // 使用 union 和类型标记
    union Payload {
        Value value;
        size_t size;
        std::string str;
        std::pair<std::string, size_t> call_var;
        std::pair<std::string, std::string> member_assign;
        FuncDefInfo func_info;
        LambdaInfo lambda_info;
        DeclaredType decl_type;
        
        Payload() {}
        ~Payload() {}
    } payload;
    
    enum PayloadType {
        PAYLOAD_NONE,
        PAYLOAD_VALUE,
        PAYLOAD_SIZE,
        PAYLOAD_STRING,
        PAYLOAD_CALL_VAR,
        PAYLOAD_MEMBER_ASSIGN,
        PAYLOAD_FUNC_INFO,
        PAYLOAD_LAMBDA_INFO,
        PAYLOAD_DECL_TYPE
    } payload_type;

    // 构造函数 - 明确区分类型
    explicit OpCode(OpCodeType t) : type(t), payload_type(PAYLOAD_NONE) {}
    
    explicit OpCode(OpCodeType t, const Value& v) : type(t), payload_type(PAYLOAD_VALUE) {
        new (&payload.value) Value(v);
    }
    
    explicit OpCode(OpCodeType t, size_t n) : type(t), payload_type(PAYLOAD_SIZE) {
        payload.size = n;
    }
    
    explicit OpCode(OpCodeType t, const char* s) : type(t), payload_type(PAYLOAD_STRING) {
        new (&payload.str) std::string(s);
    }
    
    explicit OpCode(OpCodeType t, const std::string& s) : type(t), payload_type(PAYLOAD_STRING) {
        new (&payload.str) std::string(s);
    }
    
    explicit OpCode(OpCodeType t, const std::pair<std::string, size_t>& p) 
        : type(t), payload_type(PAYLOAD_CALL_VAR) {
        new (&payload.call_var) std::pair<std::string, size_t>(p);
    }
    
    explicit OpCode(OpCodeType t, const std::pair<std::string, std::string>& p) 
        : type(t), payload_type(PAYLOAD_MEMBER_ASSIGN) {
        new (&payload.member_assign) std::pair<std::string, std::string>(p);
    }
    
    explicit OpCode(OpCodeType t, const FuncDefInfo& info) 
        : type(t), payload_type(PAYLOAD_FUNC_INFO) {
        new (&payload.func_info) FuncDefInfo(info);
    }
    
    explicit OpCode(OpCodeType t, const LambdaInfo& info) 
        : type(t), payload_type(PAYLOAD_LAMBDA_INFO) {
        new (&payload.lambda_info) LambdaInfo(info);
    }
    
    explicit OpCode(OpCodeType t, DeclaredType dt) 
        : type(t), payload_type(PAYLOAD_DECL_TYPE) {
        payload.decl_type = dt;
    }
    
    // 拷贝构造
    OpCode(const OpCode& other) : type(other.type), payload_type(other.payload_type) {
        switch (payload_type) {
            case PAYLOAD_VALUE: new (&payload.value) Value(other.payload.value); break;
            case PAYLOAD_SIZE: payload.size = other.payload.size; break;
            case PAYLOAD_STRING: new (&payload.str) std::string(other.payload.str); break;
            case PAYLOAD_CALL_VAR: new (&payload.call_var) std::pair<std::string, size_t>(other.payload.call_var); break;
            case PAYLOAD_MEMBER_ASSIGN: new (&payload.member_assign) std::pair<std::string, std::string>(other.payload.member_assign); break;
            case PAYLOAD_FUNC_INFO: new (&payload.func_info) FuncDefInfo(other.payload.func_info); break;
            case PAYLOAD_LAMBDA_INFO: new (&payload.lambda_info) LambdaInfo(other.payload.lambda_info); break;
            case PAYLOAD_DECL_TYPE: payload.decl_type = other.payload.decl_type; break;
            default: break;
        }
    }
    
    // 赋值操作符
    OpCode& operator=(const OpCode& other) {
        if (this != &other) {
            this->~OpCode();
            new (this) OpCode(other);
        }
        return *this;
    }
    
    // 移动构造
    OpCode(OpCode&& other) : type(other.type), payload_type(other.payload_type) {
        switch (payload_type) {
            case PAYLOAD_VALUE: new (&payload.value) Value(std::move(other.payload.value)); break;
            case PAYLOAD_SIZE: payload.size = other.payload.size; break;
            case PAYLOAD_STRING: new (&payload.str) std::string(std::move(other.payload.str)); break;
            case PAYLOAD_CALL_VAR: new (&payload.call_var) std::pair<std::string, size_t>(std::move(other.payload.call_var)); break;
            case PAYLOAD_MEMBER_ASSIGN: new (&payload.member_assign) std::pair<std::string, std::string>(std::move(other.payload.member_assign)); break;
            case PAYLOAD_FUNC_INFO: new (&payload.func_info) FuncDefInfo(std::move(other.payload.func_info)); break;
            case PAYLOAD_LAMBDA_INFO: new (&payload.lambda_info) LambdaInfo(std::move(other.payload.lambda_info)); break;
            case PAYLOAD_DECL_TYPE: payload.decl_type = other.payload.decl_type; break;
            default: break;
        }
        other.payload_type = PAYLOAD_NONE;
    }
    
    // 移动赋值
    OpCode& operator=(OpCode&& other) {
        if (this != &other) {
            this->~OpCode();
            new (this) OpCode(std::move(other));
        }
        return *this;
    }
    
    ~OpCode() {
        switch (payload_type) {
            case PAYLOAD_VALUE: payload.value.~Value(); break;
            case PAYLOAD_STRING: payload.str.~basic_string(); break;
            case PAYLOAD_CALL_VAR: payload.call_var.~pair(); break;
            case PAYLOAD_MEMBER_ASSIGN: payload.member_assign.~pair(); break;
            case PAYLOAD_FUNC_INFO: payload.func_info.~FuncDefInfo(); break;
            case PAYLOAD_LAMBDA_INFO: payload.lambda_info.~LambdaInfo(); break;
            default: break;
        }
    }
};

// 字节码块
class ByteCode {
public:
    ByteCode() = default;
    ByteCode(const ByteCode&) = delete; // 禁止拷贝
    ByteCode& operator=(const ByteCode&) = delete;
    ByteCode(ByteCode&&) = default;
    ByteCode& operator=(ByteCode&&) = default;

    void emit(OpCode op, std::optional<size_t> line);
    size_t currentAddress() const;
    void patchJump(size_t address, size_t target);
    
    size_t addConstant(const Value& value);
    
    std::string serialize() const;
    static ByteCode deserialize(const std::string& input);

    const std::vector<OpCode>& getInstructions() const { return instructions_; }
    std::vector<OpCode>& getInstructions() { return instructions_; }
    const std::vector<std::optional<size_t>>& getLineNumbers() const { return line_numbers_; }
    std::vector<std::optional<size_t>>& getLineNumbers() { return line_numbers_; }
    const std::vector<Value>& getConstants() const { return constants_; }
    std::vector<Value>& getConstants() { return constants_; }

private:
    std::vector<OpCode> instructions_;
    std::vector<std::optional<size_t>> line_numbers_;
    std::vector<Value> constants_;
    static bool valuesEqual(const Value& a, const Value& b);
    
    // 不使用 std::unordered_map 来避免哈希问题，改用线性搜索
    // 因为常量池通常很小
};

// 调用帧
struct CallFrame {
    size_t return_address;
    size_t base_pointer;
    std::string function_name;
    std::unordered_map<std::string, Value> locals;
    std::unordered_set<std::string> immutable_locals;
};

// 虚拟机
class VM {
public:
    explicit VM(std::shared_ptr<std::unordered_map<std::string, Value>> globals);

    void load(ByteCode bytecode);
    Result<std::optional<Value>> run();

    std::pair<size_t, size_t> getCacheStats() const;

private:
    ByteCode bytecode_;
    size_t ip_ = 0;
    std::vector<Value> stack_;
    std::vector<CallFrame> call_stack_;
    
    std::shared_ptr<std::unordered_map<std::string, Value>> globals_;
    std::unordered_map<std::string, Value> locals_;
    std::unordered_set<std::string> immutable_globals_;
    std::unordered_set<std::string> immutable_locals_;
    
    std::vector<std::pair<size_t, size_t>> loop_stack_;
    std::unordered_map<std::string, FuncDefInfo> functions_;
    
    struct InlineCache {
        std::string member;
        size_t hits = 0;
        size_t misses = 0;
    };
    std::unordered_map<size_t, InlineCache> member_cache_;
    
    bool halted_ = false;
    size_t recursion_depth_ = 0;
    static constexpr size_t MAX_RECURSION_DEPTH = 4000;

    void executeInstructionAt(size_t ip);
    
    template<typename F>
    void binaryOp(F&& f);
    
    Value getVariable(const std::string& name) const;
    void setVariable(const std::string& name, const Value& value);
    void ensureMutable(const std::string& name) const;
    void setVariableChecked(const std::string& name, const Value& value);
    
    Value convertToType(const Value& val, DeclaredType dtype) const;
};

} // namespace rumina
