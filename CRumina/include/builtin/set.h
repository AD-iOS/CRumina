#pragma once

#include "../value.h"
#include <vector>
#include <memory>

namespace rumina {
namespace builtin {
namespace set {

// Set 构造函数和基础操作
Value set_constructor(const std::vector<Value>& args);
Value set_get(const std::vector<Value>& args);
Value set_main_value(const std::vector<Value>& args);
Value set_get_real(const std::vector<Value>& args);

// Set 算术运算
Value set_to_add(const std::vector<Value>& args);
Value set_to_sub(const std::vector<Value>& args);
Value set_to_multiply(const std::vector<Value>& args);
Value set_to_divide(const std::vector<Value>& args);
Value set_to_pow(const std::vector<Value>& args);
Value set_to_sqrt(const std::vector<Value>& args);

// Set 三角函数
Value set_to_sin(const std::vector<Value>& args);
Value set_to_cos(const std::vector<Value>& args);
Value set_to_tangent(const std::vector<Value>& args);

} // namespace set
} // namespace builtin
} // namespace rumina
