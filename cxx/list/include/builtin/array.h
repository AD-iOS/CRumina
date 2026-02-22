#pragma once

#include "../value.h"
#include <vector>
#include <memory>

namespace rumina {
namespace builtin {
namespace array {

// 数组函数
Value foreach(const std::vector<Value>& args);
Value map(const std::vector<Value>& args);
Value filter(const std::vector<Value>& args);
Value reduce(const std::vector<Value>& args);
Value push(const std::vector<Value>& args);
Value pop(const std::vector<Value>& args);
Value range(const std::vector<Value>& args);
Value concat(const std::vector<Value>& args);
Value dot(const std::vector<Value>& args);
Value norm(const std::vector<Value>& args);
Value cross(const std::vector<Value>& args);
Value det(const std::vector<Value>& args);

// 辅助函数
double calculateDeterminant(const std::vector<std::vector<double>>& matrix);

} // namespace array
} // namespace builtin
} // namespace rumina
