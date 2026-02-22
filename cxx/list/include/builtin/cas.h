#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace cas {

// 计算机代数系统函数
Value cas_parse(const std::vector<Value>& args);
Value cas_differentiate(const std::vector<Value>& args);
Value cas_solve_linear(const std::vector<Value>& args);
Value cas_evaluate_at(const std::vector<Value>& args);
Value cas_store(const std::vector<Value>& args);
Value cas_load(const std::vector<Value>& args);
Value cas_numerical_derivative(const std::vector<Value>& args);
Value cas_integrate(const std::vector<Value>& args);
Value cas_definite_integral(const std::vector<Value>& args);

} // namespace cas
} // namespace builtin
} // namespace rumina
