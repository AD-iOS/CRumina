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

// 在 VM 类定义之前添加错误常量
namespace vm_constants {
/*
    constexpr const char* ERR_STACK_UNDERFLOW = "Stack underflow";
    constexpr const char* ERR_INVALID_CONST_INDEX = "Invalid constant pool index";
    constexpr const char* ERR_ARRAY_INDEX_MUST_BE_INT = "Array index must be an integer";
    constexpr const char* ERR_STRING_INDEX_MUST_BE_INT = "String index must be an integer";
    constexpr const char* ERR_CANNOT_INDEX_TYPE = "Cannot index type";
    constexpr const char* ERR_CANNOT_CALL_TYPE = "Cannot call type";
    constexpr const char* ERR_BREAK_OUTSIDE_LOOP = "Break outside of loop";
    constexpr const char* ERR_CONTINUE_OUTSIDE_LOOP = "Continue outside of loop";
    constexpr const char* ERR_LAMBDA_ID_NOT_FOUND = "Lambda ID not found";
*/

// 改为声明
    extern const char* ERR_STACK_UNDERFLOW;
    extern const char* ERR_INVALID_CONST_INDEX;
    extern const char* ERR_ARRAY_INDEX_MUST_BE_INT;
    extern const char* ERR_STRING_INDEX_MUST_BE_INT;
    extern const char* ERR_CANNOT_INDEX_TYPE;
    extern const char* ERR_CANNOT_CALL_TYPE;
    extern const char* ERR_BREAK_OUTSIDE_LOOP;
    extern const char* ERR_CONTINUE_OUTSIDE_LOOP;
    extern const char* ERR_LAMBDA_ID_NOT_FOUND;
} /* namespace vm_constants */

// InlineCache

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
    
    // 添加 valuesEqual 声明
    static bool valuesEqual(const Value& a, const Value& b);
    
    // 常量缓存结构
    struct ConstantKey {
        enum Type { INT, FLOAT, BOOL, STRING, NULL_ } type;
        union {
            int64_t int_val;
            double float_val;
            bool bool_val;
        };
        std::string str_val;
        
        ConstantKey(int64_t i) : type(INT), int_val(i) {}
        ConstantKey(double f) : type(FLOAT), float_val(f) {}
        ConstantKey(bool b) : type(BOOL), bool_val(b) {}
        ConstantKey(const std::string& s) : type(STRING), str_val(s) {}
        ConstantKey(std::nullptr_t) : type(NULL_) {}
        
        bool operator==(const ConstantKey& other) const {
            if (type != other.type) return false;
            switch (type) {
                case INT: return int_val == other.int_val;
                case FLOAT: return std::abs(float_val - other.float_val) < 1e-10;
                case BOOL: return bool_val == other.bool_val;
                case STRING: return str_val == other.str_val;
                case NULL_: return true;
            }
            return false;
        }
    };
    
    struct ConstantKeyHash {
        size_t operator()(const ConstantKey& key) const {
            size_t h = std::hash<int>()(static_cast<int>(key.type));
            switch (key.type) {
                case ConstantKey::INT:
                    return h ^ std::hash<int64_t>()(key.int_val);
                case ConstantKey::FLOAT:
                    return h ^ std::hash<double>()(key.float_val);
                case ConstantKey::BOOL:
                    return h ^ std::hash<bool>()(key.bool_val);
                case ConstantKey::STRING:
                    return h ^ std::hash<std::string>()(key.str_val);
                case ConstantKey::NULL_:
                    return h;
            }
            return h;
        }
    };
    
    std::unordered_map<ConstantKey, size_t, ConstantKeyHash> constant_cache_;
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
        // size_t hits = 0;
        // size_t misses = 0;

        size_t hits;
        size_t misses;
    
        InlineCache(const std::string& m) : member(m), hits(0), misses(0) {}
        InlineCache(const std::string& m, size_t h, size_t ms) : member(m), hits(h), misses(ms) {}

    };
    std::unordered_map<size_t, InlineCache> member_cache_;
    // std::unordered_map<size_t, InlineCache> member_cache_;
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
