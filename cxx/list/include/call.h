#pragma once

#include "value.h"
#include "ast.h"
#include "interpreter.h"
#include <vector>

namespace rumina {

// 函数调用实现（供解释器内部使用）
namespace call {

// 函数调用
Value call_function(Interpreter* interp, const Value& func, std::vector<Value> args);

// 方法调用（带self注入）
Value call_method(Interpreter* interp, const Value& func, const Value& self_obj, 
                  std::vector<Value> args);

// 高阶函数处理
Value handle_foreach(Interpreter* interp, const std::vector<Value>& args);
Value handle_map(Interpreter* interp, const std::vector<Value>& args);
Value handle_filter(Interpreter* interp, const std::vector<Value>& args);
Value handle_reduce(Interpreter* interp, const std::vector<Value>& args);

} // namespace call
} // namespace rumina
