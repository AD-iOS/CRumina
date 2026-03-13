#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"

namespace rumina {

// 表达式求值实现（供解释器内部使用）
namespace expr {

Value eval_expr(Interpreter* interp, const Expr* expr);

} // namespace expr
} // namespace rumina
