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
