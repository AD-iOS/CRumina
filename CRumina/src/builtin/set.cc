#include <builtin/set.h>
#include <builtin/math.h>
#include <value_ops.h>

namespace rumina {
namespace builtin {
namespace set {

// Set 构造函数
Value set_constructor(const std::vector<Value>& args) {
    if (args.empty()) {
        throw std::runtime_error("Set constructor expects at least 1 argument");
    }

    const Value& val = args[0];

    if (val.getType() == Value::Type::Array) {
        auto arr = val.getArray();
        return Value::makeArray(std::make_shared<std::vector<Value>>(*arr));
    } else if (val.getType() == Value::Type::Array) {
        return val; // 已经是 Set
    } else {
        std::vector<Value> values;
        values.push_back(val);
        return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(values)));
    }
}

// set_get(index)
Value set_get(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("set_get expects 2 arguments (set, index)");
    }

    const Value& set_val = args[0];
    const Value& index_val = args[1];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_get expects set, got " + set_val.typeName());
    }

    if (index_val.getType() != Value::Type::Int) {
        throw std::runtime_error("set_get expects int index, got " + index_val.typeName());
    }

    auto set = set_val.getArray();
    int64_t idx = index_val.getInt();

    if (idx < 0 || static_cast<size_t>(idx) >= set->size()) {
        throw std::runtime_error("Index " + std::to_string(idx) + 
                                 " out of bounds for set of size " + 
                                 std::to_string(set->size()));
    }

    return (*set)[idx];
}

// set_main_value()
Value set_main_value(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("set_main_value expects 1 argument (set)");
    }

    const Value& set_val = args[0];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_main_value expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    if (set->empty()) {
        throw std::runtime_error("Cannot get main value from empty set");
    }

    return (*set)[0];
}

// set_get_real()
Value set_get_real(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("set_get_real expects 1 argument (set)");
    }

    const Value& set_val = args[0];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_get_real expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> real_values;

    for (const auto& v : *set) {
        if (v.getType() == Value::Type::Int ||
            v.getType() == Value::Type::Float ||
            v.getType() == Value::Type::BigInt ||
            v.getType() == Value::Type::Rational) {
            real_values.push_back(v);
        }
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(real_values)));
}

// set_to_add(value)
Value set_to_add(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("set_to_add expects 2 arguments (set, value)");
    }

    const Value& set_val = args[0];
    const Value& operand = args[1];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_add expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    if (operand.getType() == Value::Type::Array) {
        auto other_set = operand.getArray();
        for (const auto& v1 : *set) {
            for (const auto& v2 : *other_set) {
                auto result = value_binary_op(v1, BinOp::Add, v2);
                if (result.is_error()) throw std::runtime_error(result.error());
                results.push_back(result.value());
            }
        }
    } else {
        for (const auto& v : *set) {
            auto result = value_binary_op(v, BinOp::Add, operand);
            if (result.is_error()) throw std::runtime_error(result.error());
            results.push_back(result.value());
        }
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_sub(value)
Value set_to_sub(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("set_to_sub expects 2 arguments (set, value)");
    }

    const Value& set_val = args[0];
    const Value& operand = args[1];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_sub expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    if (operand.getType() == Value::Type::Array) {
        auto other_set = operand.getArray();
        for (const auto& v1 : *set) {
            for (const auto& v2 : *other_set) {
                auto result = value_binary_op(v1, BinOp::Sub, v2);
                if (result.is_error()) throw std::runtime_error(result.error());
                results.push_back(result.value());
            }
        }
    } else {
        for (const auto& v : *set) {
            auto result = value_binary_op(v, BinOp::Sub, operand);
            if (result.is_error()) throw std::runtime_error(result.error());
            results.push_back(result.value());
        }
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_multiply(value)
Value set_to_multiply(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("set_to_multiply expects 2 arguments (set, value)");
    }

    const Value& set_val = args[0];
    const Value& operand = args[1];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_multiply expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    if (operand.getType() == Value::Type::Array) {
        auto other_set = operand.getArray();
        for (const auto& v1 : *set) {
            for (const auto& v2 : *other_set) {
                auto result = value_binary_op(v1, BinOp::Mul, v2);
                if (result.is_error()) throw std::runtime_error(result.error());
                results.push_back(result.value());
            }
        }
    } else {
        for (const auto& v : *set) {
            auto result = value_binary_op(v, BinOp::Mul, operand);
            if (result.is_error()) throw std::runtime_error(result.error());
            results.push_back(result.value());
        }
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_divide(value)
Value set_to_divide(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("set_to_divide expects 2 arguments (set, value)");
    }

    const Value& set_val = args[0];
    const Value& operand = args[1];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_divide expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    if (operand.getType() == Value::Type::Array) {
        auto other_set = operand.getArray();
        for (const auto& v1 : *set) {
            for (const auto& v2 : *other_set) {
                auto result = value_binary_op(v1, BinOp::Div, v2);
                if (result.is_error()) throw std::runtime_error(result.error());
                results.push_back(result.value());
            }
        }
    } else {
        for (const auto& v : *set) {
            auto result = value_binary_op(v, BinOp::Div, operand);
            if (result.is_error()) throw std::runtime_error(result.error());
            results.push_back(result.value());
        }
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_pow(exponent)
Value set_to_pow(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("set_to_pow expects 2 arguments (set, exponent)");
    }

    const Value& set_val = args[0];
    const Value& exponent = args[1];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_pow expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    if (exponent.getType() == Value::Type::Array) {
        auto other_set = exponent.getArray();
        for (const auto& v1 : *set) {
            for (const auto& v2 : *other_set) {
                auto result = value_binary_op(v1, BinOp::Pow, v2);
                if (result.is_error()) throw std::runtime_error(result.error());
                results.push_back(result.value());
            }
        }
    } else {
        for (const auto& v : *set) {
            auto result = value_binary_op(v, BinOp::Pow, exponent);
            if (result.is_error()) throw std::runtime_error(result.error());
            results.push_back(result.value());
        }
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_sqrt()
Value set_to_sqrt(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("set_to_sqrt expects 1 argument (set)");
    }

    const Value& set_val = args[0];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_sqrt expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    for (const auto& v : *set) {
        std::vector<Value> sqrt_args = {v};
        auto result = math::sqrt(sqrt_args);
        results.push_back(result);
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_sin()
Value set_to_sin(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("set_to_sin expects 1 argument (set)");
    }

    const Value& set_val = args[0];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_sin expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    for (const auto& v : *set) {
        std::vector<Value> sin_args = {v};
        auto result = math::sin(sin_args);
        results.push_back(result);
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_cos()
Value set_to_cos(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("set_to_cos expects 1 argument (set)");
    }

    const Value& set_val = args[0];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_cos expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    for (const auto& v : *set) {
        std::vector<Value> cos_args = {v};
        auto result = math::cos(cos_args);
        results.push_back(result);
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

// set_to_tangent()
Value set_to_tangent(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("set_to_tangent expects 1 argument (set)");
    }

    const Value& set_val = args[0];

    if (set_val.getType() != Value::Type::Array) {
        throw std::runtime_error("set_to_tangent expects set, got " + set_val.typeName());
    }

    auto set = set_val.getArray();
    std::vector<Value> results;

    for (const auto& v : *set) {
        std::vector<Value> tan_args = {v};
        auto result = math::tan(tan_args);
        results.push_back(result);
    }

    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(results)));
}

} // namespace set
} // namespace builtin
} // namespace rumina
