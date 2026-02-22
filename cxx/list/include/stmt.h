#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"

namespace rumina {

// 语句执行实现（供解释器内部使用）
namespace stmt {

void execute_stmt(Interpreter* interp, const Stmt* stmt);

} // namespace stmt
} // namespace rumina
