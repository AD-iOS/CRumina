#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace math {

// 数学函数
Value sqrt(const std::vector<Value>& args);
Value pi(const std::vector<Value>& args);
Value e(const std::vector<Value>& args);
Value sin(const std::vector<Value>& args);
Value cos(const std::vector<Value>& args);
Value tan(const std::vector<Value>& args);
Value exp(const std::vector<Value>& args);
Value abs_fn(const std::vector<Value>& args);
Value log(const std::vector<Value>& args);
Value ln(const std::vector<Value>& args);
Value logbase(const std::vector<Value>& args);
Value factorial(const std::vector<Value>& args);

// 复数函数
Value arg(const std::vector<Value>& args);
Value conj(const std::vector<Value>& args);
Value re(const std::vector<Value>& args);
Value im(const std::vector<Value>& args);

} // namespace math
} // namespace builtin
} // namespace rumina
