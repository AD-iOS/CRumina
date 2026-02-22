#pragma once

#include "value.h"
#include "ast.h"

namespace rumina {

// 类型转换函数（供解释器内部使用）
namespace convert {

Value convert_to_int(const Value& val);
Value convert_to_float(const Value& val);
Value convert_to_bool(const Value& val);
Value convert_to_string(const Value& val);
Value convert_to_rational(const Value& val);
Value convert_to_irrational(const Value& val);
Value convert_to_complex(const Value& val);
Value convert_to_array(const Value& val);
Value convert_to_bigint(const Value& val);

Value convert_to_declared_type(const Value& val, DeclaredType dtype);

} // namespace convert
} // namespace rumina
