#pragma once

#include "../value.h"
#include "../ast.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace rumina {
namespace builtin {

// 注册所有内置函数
void register_builtins(std::unordered_map<std::string, Value>& globals);

// 类型转换函数（供解释器和VM使用）
Value convert_to_declared_type(const Value& val, DeclaredType dtype);
Value convert_to_int(const std::vector<Value>& args);
Value convert_to_float(const std::vector<Value>& args);
Value convert_to_bool(const std::vector<Value>& args);
Value convert_to_string(const std::vector<Value>& args);
Value convert_to_rational(const std::vector<Value>& args);
Value convert_to_irrational(const std::vector<Value>& args);
Value convert_to_complex(const std::vector<Value>& args);
Value convert_to_array(const std::vector<Value>& args);
Value convert_to_bigint(const std::vector<Value>& args);

} // namespace builtin
} // namespace rumina
