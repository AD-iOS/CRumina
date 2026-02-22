#pragma once

#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char** environ;
#endif

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace env {

// 环境变量模块创建
Value create_env_module();

// 环境变量操作
Value env_get(const std::vector<Value>& args);
Value env_set(const std::vector<Value>& args);
Value env_has(const std::vector<Value>& args);
Value env_remove(const std::vector<Value>& args);
Value env_all(const std::vector<Value>& args);
Value env_keys(const std::vector<Value>& args);

} // namespace env
} // namespace builtin
} // namespace rumina
