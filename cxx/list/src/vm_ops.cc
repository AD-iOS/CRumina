#include <vm_ops.h>
#include <value_ops.h>

namespace rumina {

Result<Value> ValueVMOps::vm_add(const Value& other) const {
    return value_binary_op(value_, BinOp::Add, other);
}

Result<Value> ValueVMOps::vm_sub(const Value& other) const {
    return value_binary_op(value_, BinOp::Sub, other);
}

Result<Value> ValueVMOps::vm_mul(const Value& other) const {
    return value_binary_op(value_, BinOp::Mul, other);
}

Result<Value> ValueVMOps::vm_div(const Value& other) const {
    return value_binary_op(value_, BinOp::Div, other);
}

Result<Value> ValueVMOps::vm_mod(const Value& other) const {
    return value_binary_op(value_, BinOp::Mod, other);
}

Result<Value> ValueVMOps::vm_pow(const Value& other) const {
    return value_binary_op(value_, BinOp::Pow, other);
}

Result<Value> ValueVMOps::vm_neg() const {
    return value_unary_op(UnaryOp::Neg, value_);
}

Result<Value> ValueVMOps::vm_not() const {
    return value_unary_op(UnaryOp::Not, value_);
}

Result<Value> ValueVMOps::vm_factorial() const {
    return value_unary_op(UnaryOp::Factorial, value_);
}

Result<Value> ValueVMOps::vm_eq(const Value& other) const {
    return value_binary_op(value_, BinOp::Equal, other);
}

Result<Value> ValueVMOps::vm_neq(const Value& other) const {
    return value_binary_op(value_, BinOp::NotEqual, other);
}

Result<Value> ValueVMOps::vm_gt(const Value& other) const {
    return value_binary_op(value_, BinOp::Greater, other);
}

Result<Value> ValueVMOps::vm_gte(const Value& other) const {
    return value_binary_op(value_, BinOp::GreaterEq, other);
}

Result<Value> ValueVMOps::vm_lt(const Value& other) const {
    return value_binary_op(value_, BinOp::Less, other);
}

Result<Value> ValueVMOps::vm_lte(const Value& other) const {
    return value_binary_op(value_, BinOp::LessEq, other);
}

Result<Value> ValueVMOps::vm_and(const Value& other) const {
    return value_binary_op(value_, BinOp::And, other);
}

Result<Value> ValueVMOps::vm_or(const Value& other) const {
    return value_binary_op(value_, BinOp::Or, other);
}

} // namespace rumina
