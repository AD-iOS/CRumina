#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {

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

// 别名（无cas_前缀）
inline Value parse(const std::vector<Value>& args) { return cas_parse(args); }
inline Value differentiate(const std::vector<Value>& args) { return cas_differentiate(args); }
inline Value solve_linear(const std::vector<Value>& args) { return cas_solve_linear(args); }
inline Value evaluate_at(const std::vector<Value>& args) { return cas_evaluate_at(args); }
inline Value store(const std::vector<Value>& args) { return cas_store(args); }
inline Value load(const std::vector<Value>& args) { return cas_load(args); }
inline Value numerical_derivative(const std::vector<Value>& args) { return cas_numerical_derivative(args); }
inline Value integrate(const std::vector<Value>& args) { return cas_integrate(args); }
inline Value definite_integral(const std::vector<Value>& args) { return cas_definite_integral(args); }

} // namespace builtin
} // namespace rumina
