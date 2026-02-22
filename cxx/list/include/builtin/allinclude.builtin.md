```array.h
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
```

```buffer.h
#pragma once

#include "../value.h"
#include <vector>
#include <string>

namespace rumina {
namespace builtin {
namespace buffer {

// Buffer模块创建
Value create_buffer_module();

// Buffer操作
Value buffer_alloc(const std::vector<Value>& args);
Value buffer_from(const std::vector<Value>& args);
Value buffer_concat(const std::vector<Value>& args);
Value buffer_length(const std::vector<Value>& args);
Value buffer_get(const std::vector<Value>& args);
Value buffer_set(const std::vector<Value>& args);
Value buffer_slice(const std::vector<Value>& args);
Value buffer_to_text(const std::vector<Value>& args);
Value buffer_to_hex(const std::vector<Value>& args);
Value buffer_to_base64(const std::vector<Value>& args);
Value buffer_to_base64_url(const std::vector<Value>& args);
Value buffer_copy(const std::vector<Value>& args);
Value buffer_fill(const std::vector<Value>& args);
Value buffer_index_of(const std::vector<Value>& args);
Value buffer_includes(const std::vector<Value>& args);
Value buffer_equals(const std::vector<Value>& args);
Value buffer_compare(const std::vector<Value>& args);
Value buffer_subarray(const std::vector<Value>& args);

// 辅助函数
Value new_buffer_from_bytes(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> buffer_to_bytes(const Value& value);
std::vector<uint8_t> decode_hex_nodeish(const std::string& s);
std::vector<uint8_t> decode_base64_nodeish(const std::string& s);

} // namespace buffer
} // namespace builtin
} // namespace rumina
```

```builtin.h
#pragma once

#include "../value.h"
#include "../ast.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace rumina {
namespace builtin {

// 注册所有内置函数
void register_builtins(std::unordered_map<std::string, Value>& globals);

// 类型转换函数（供解释器和VM使用）
Value convert_to_declared_type(const Value& val, DeclaredType dtype);
Value convert_to_int(const std::vector<Value>& args);
Value convert_to_float(const std::vector<Value>& args);
Value convert_to_bool(const std::vector<Value>& args);
Value convert_to_string(const std::vector<Value>& args);
Value convert_to_rational(const std::vector<Value>& args);
Value convert_to_irrational(const std::vector<Value>& args);
Value convert_to_complex(const std::vector<Value>& args);
Value convert_to_array(const std::vector<Value>& args);
Value convert_to_bigint(const std::vector<Value>& args);

} // namespace builtin
} // namespace rumina
```

```cas.h
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
```

```env.h
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
```

```fs.h
#pragma once

#include "../value.h"
#include <vector>
#include <string>

namespace rumina {
namespace builtin {
namespace fs {

// 文件系统模块创建
Value create_fs_module();

// 文件系统操作
Value fs_read_text(const std::vector<Value>& args);
Value fs_read_bytes(const std::vector<Value>& args);
Value fs_write_text(const std::vector<Value>& args);
Value fs_write_bytes(const std::vector<Value>& args);
Value fs_append(const std::vector<Value>& args);
Value fs_exists(const std::vector<Value>& args);
Value fs_is_file(const std::vector<Value>& args);
Value fs_is_dir(const std::vector<Value>& args);
Value fs_stat(const std::vector<Value>& args);
Value fs_make_dir(const std::vector<Value>& args);
Value fs_make_dir_all(const std::vector<Value>& args);
Value fs_read_dir(const std::vector<Value>& args);
Value fs_remove(const std::vector<Value>& args);
Value fs_remove_all(const std::vector<Value>& args);
Value fs_rename(const std::vector<Value>& args);
Value fs_copy(const std::vector<Value>& args);
Value fs_realpath(const std::vector<Value>& args);
Value fs_read_link(const std::vector<Value>& args);
Value fs_link(const std::vector<Value>& args);
Value fs_symlink(const std::vector<Value>& args);
Value fs_chmod(const std::vector<Value>& args);

} // namespace fs
} // namespace builtin
} // namespace rumina
```

```math.h
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
```

```path.h
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
```

```process.h
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
```

```random.h
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
```

```stream.h
#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace stream {

// 流模块创建
Value create_stream_module();

// 流操作
Value stream_open_read(const std::vector<Value>& args);
Value stream_open_write(const std::vector<Value>& args);

// ReadStream方法
Value read_stream_read_bytes(const std::vector<Value>& args);
Value read_stream_read_until(const std::vector<Value>& args);
Value read_stream_read_all(const std::vector<Value>& args);
Value read_stream_seek(const std::vector<Value>& args);
Value read_stream_tell(const std::vector<Value>& args);
Value read_stream_is_closed(const std::vector<Value>& args);
Value read_stream_close(const std::vector<Value>& args);

// WriteStream方法
Value write_stream_write_bytes(const std::vector<Value>& args);
Value write_stream_write_text(const std::vector<Value>& args);
Value write_stream_flush(const std::vector<Value>& args);
Value write_stream_seek(const std::vector<Value>& args);
Value write_stream_tell(const std::vector<Value>& args);
Value write_stream_is_closed(const std::vector<Value>& args);
Value write_stream_close(const std::vector<Value>& args);

} // namespace stream
} // namespace builtin
} // namespace rumina
```

```string.h
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
```

```time.h
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
```

```utils.h
#pragma once

#include "../value.h"
#include <vector>

namespace rumina {
namespace builtin {
namespace utils {

// 工具函数
Value print(const std::vector<Value>& args);
Value input(const std::vector<Value>& args);
Value typeof_fn(const std::vector<Value>& args);
Value size(const std::vector<Value>& args);
Value tostring(const std::vector<Value>& args);
Value to_string(const std::vector<Value>& args);
Value exit(const std::vector<Value>& args);
Value new_fn(const std::vector<Value>& args);
Value same(const std::vector<Value>& args);
Value setattr(const std::vector<Value>& args);
Value update(const std::vector<Value>& args);
Value fraction(const std::vector<Value>& args);
Value decimal(const std::vector<Value>& args);
Value assert_fn(const std::vector<Value>& args);

// 类型转换函数
Value to_int(const std::vector<Value>& args);
Value to_float(const std::vector<Value>& args);
Value to_bool(const std::vector<Value>& args);
Value to_string_fn(const std::vector<Value>& args);
Value to_rational(const std::vector<Value>& args);
Value to_complex(const std::vector<Value>& args);

} // namespace utils
} // namespace builtin
} // namespace rumina
```

