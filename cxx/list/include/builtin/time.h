#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {

// 时间模块创建
Value create_time_module();

// 时间函数
Value time_now(const std::vector<Value>& args);
Value time_hrtime_ms(const std::vector<Value>& args);
Value time_sleep(const std::vector<Value>& args);
Value time_start_timer(const std::vector<Value>& args);

// Timer方法
Value timer_elapsed_ms(const std::vector<Value>& args);
Value timer_elapsed_sec(const std::vector<Value>& args);

} // namespace builtin
} // namespace rumina
