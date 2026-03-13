#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace path {

// 路径模块创建
Value create_path_module();

// 路径操作
Value path_join(const std::vector<Value>& args);
Value path_basename(const std::vector<Value>& args);
Value path_dirname(const std::vector<Value>& args);
Value path_extname(const std::vector<Value>& args);
Value path_is_absolute(const std::vector<Value>& args);
Value path_normalize(const std::vector<Value>& args);
Value path_resolve(const std::vector<Value>& args);
Value path_relative(const std::vector<Value>& args);
Value path_parse(const std::vector<Value>& args);
Value path_format(const std::vector<Value>& args);

} // namespace path
} // namespace builtin
} // namespace rumina
