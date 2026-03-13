#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace utils {

// 工具函数
Value print(const std::vector<Value>& args);
Value input(const std::vector<Value>& args);
Value typeof_fn(const std::vector<Value>& args);
Value size(const std::vector<Value>& args);
Value tostring(const std::vector<Value>& args);
Value to_string(const std::vector<Value>& args);
Value exit(const std::vector<Value>& args);
Value new_fn(const std::vector<Value>& args);
Value same(const std::vector<Value>& args);
Value setattr(const std::vector<Value>& args);
Value update(const std::vector<Value>& args);
Value fraction(const std::vector<Value>& args);
Value decimal(const std::vector<Value>& args);
Value assert_fn(const std::vector<Value>& args);

// 类型转换函数
Value to_int(const std::vector<Value>& args);
Value to_float(const std::vector<Value>& args);
Value to_bool(const std::vector<Value>& args);
Value to_string_fn(const std::vector<Value>& args);
Value to_rational(const std::vector<Value>& args);
Value to_complex(const std::vector<Value>& args);

} // namespace utils
} // namespace builtin
} // namespace rumina
