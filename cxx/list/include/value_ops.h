#pragma once

#include "value.h"
#include "ast.h"
#include "result.h"
#include <memory>

namespace rumina {

// 并行幂运算阈值
constexpr uint32_t PARALLEL_POW_THRESHOLD = 10000;

// 二进制操作
Result<Value> value_binary_op(const Value& left, BinOp op, const Value& right);

// 一元操作
Result<Value> value_unary_op(UnaryOp op, const Value& val);

// 大数幂运算优化
BigInt bigint_pow_optimized(const BigInt& base, uint32_t exponent);

// 并行大数幂运算
BigInt bigint_pow_parallel(const BigInt& base, uint32_t exponent);

// 幂运算符号处理
Result<Value> compute_power(double base, double exponent);

// 无理数乘法
Result<Value> multiply_irrationals(const IrrationalValue& a, const IrrationalValue& b);

} // namespace rumina
