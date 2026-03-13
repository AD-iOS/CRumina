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
