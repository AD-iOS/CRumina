#pragma once

#include "value.h"
#include "ast.h"
#include "result.h"
#include <functional>

namespace rumina {

// VM操作接口类
class VMOperations {
public:
    virtual ~VMOperations() = default;

    // 算术运算
    virtual Result<Value> vm_add(const Value& other) const = 0;
    virtual Result<Value> vm_sub(const Value& other) const = 0;
    virtual Result<Value> vm_mul(const Value& other) const = 0;
    virtual Result<Value> vm_div(const Value& other) const = 0;
    virtual Result<Value> vm_mod(const Value& other) const = 0;
    virtual Result<Value> vm_pow(const Value& other) const = 0;
    
    // 一元运算
    virtual Result<Value> vm_neg() const = 0;
    virtual Result<Value> vm_not() const = 0;
    virtual Result<Value> vm_factorial() const = 0;

    // 比较运算
    virtual Result<Value> vm_eq(const Value& other) const = 0;
    virtual Result<Value> vm_neq(const Value& other) const = 0;
    virtual Result<Value> vm_gt(const Value& other) const = 0;
    virtual Result<Value> vm_gte(const Value& other) const = 0;
    virtual Result<Value> vm_lt(const Value& other) const = 0;
    virtual Result<Value> vm_lte(const Value& other) const = 0;

    // 逻辑运算
    virtual Result<Value> vm_and(const Value& other) const = 0;
    virtual Result<Value> vm_or(const Value& other) const = 0;
};

// Value类的VM操作实现
class ValueVMOps : public VMOperations {
private:
    const Value& value_;

public:
    explicit ValueVMOps(const Value& v) : value_(v) {}

    Result<Value> vm_add(const Value& other) const override;
    Result<Value> vm_sub(const Value& other) const override;
    Result<Value> vm_mul(const Value& other) const override;
    Result<Value> vm_div(const Value& other) const override;
    Result<Value> vm_mod(const Value& other) const override;
    Result<Value> vm_pow(const Value& other) const override;

    Result<Value> vm_neg() const override;
    Result<Value> vm_not() const override;
    Result<Value> vm_factorial() const override;

    Result<Value> vm_eq(const Value& other) const override;
    Result<Value> vm_neq(const Value& other) const override;
    Result<Value> vm_gt(const Value& other) const override;
    Result<Value> vm_gte(const Value& other) const override;
    Result<Value> vm_lt(const Value& other) const override;
    Result<Value> vm_lte(const Value& other) const override;

    Result<Value> vm_and(const Value& other) const override;
    Result<Value> vm_or(const Value& other) const override;
};

// 辅助函数
inline ValueVMOps get_vm_ops(const Value& value) {
    return ValueVMOps(value);
}

} // namespace rumina
