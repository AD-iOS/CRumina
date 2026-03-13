#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace process {

// 初始化函数，需要在 main 函数中调用
void init_process_args(int argc, char* argv[]);

// 进程模块创建
Value create_process_module();

// 进程操作
Value process_args(const std::vector<Value>& args);
Value process_cwd(const std::vector<Value>& args);
Value process_set_cwd(const std::vector<Value>& args);
Value process_pid(const std::vector<Value>& args);
Value process_exit(const std::vector<Value>& args);
Value process_platform(const std::vector<Value>& args);
Value process_arch(const std::vector<Value>& args);
Value process_version(const std::vector<Value>& args);
Value process_exec_path(const std::vector<Value>& args);

} // namespace process
} // namespace builtin
} // namespace rumina
