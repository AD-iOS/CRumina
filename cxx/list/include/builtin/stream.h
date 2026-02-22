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
