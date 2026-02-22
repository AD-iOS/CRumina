#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace string {

// 字符串函数
Value concat(const std::vector<Value>& args);
Value length(const std::vector<Value>& args);
Value char_at(const std::vector<Value>& args);
Value at(const std::vector<Value>& args);
Value find(const std::vector<Value>& args);
Value sub(const std::vector<Value>& args);
Value cat(const std::vector<Value>& args);
Value replace_by_index(const std::vector<Value>& args);

} // namespace string
} // namespace builtin
} // namespace rumina
