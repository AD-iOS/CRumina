#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace random_ns {
// 随机数函数
Value rand(const std::vector<Value>& args);
Value randint(const std::vector<Value>& args);
Value random(const std::vector<Value>& args);

} // namespace random_ns
} // namespace builtin
} // namespace rumina
