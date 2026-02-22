#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"

namespace rumina {

// 运算符实现（供解释器内部使用）
namespace operators {

Value compute_power(Interpreter* interp, double base, double exponent);
Value multiply_irrationals(Interpreter* interp, const IrrationalValue& a, const IrrationalValue& b);

// 二元运算
Value eval_binary_op(Interpreter* interp, const Value& left, BinOp op, const Value& right);

// 一元运算
Value eval_unary_op(Interpreter* interp, UnaryOp op, const Value& val);

} // namespace operators
} // namespace rumina
