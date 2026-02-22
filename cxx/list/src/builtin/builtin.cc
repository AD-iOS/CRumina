#include <builtin/builtin.h>
#include <builtin/math.h>
#include <builtin/utils.h>
#include <builtin/string.h>
#include <builtin/array.h>
#include <builtin/fs.h>
#include <builtin/path.h>
#include <builtin/env.h>
#include <builtin/process.h>
#include <builtin/stream.h>
#include <builtin/stream.h>
#include <builtin/buffer.h>
#include <builtin/cas.h>
#include <builtin/random.h>
#include <builtin/time.h>

#include <algorithm>
#include <numeric>
#include <cstdlib>

namespace rumina {
namespace builtin {

void register_builtins(std::unordered_map<std::string, Value>& globals) {
    // 数学函数
    globals["sqrt"] = Value::makeNativeFunction("sqrt", math::sqrt);
    globals["pi"] = Value::makeNativeFunction("pi", math::pi);
    globals["e"] = Value::makeNativeFunction("e", math::e);
    globals["sin"] = Value::makeNativeFunction("sin", math::sin);
    globals["cos"] = Value::makeNativeFunction("cos", math::cos);
    globals["tan"] = Value::makeNativeFunction("tan", math::tan);
    globals["exp"] = Value::makeNativeFunction("exp", math::exp);
    globals["abs"] = Value::makeNativeFunction("abs", math::abs_fn);
    globals["log"] = Value::makeNativeFunction("log", math::log);
    globals["ln"] = Value::makeNativeFunction("ln", math::ln);
    globals["logBASE"] = Value::makeNativeFunction("logBASE", math::logbase);
    globals["factorial"] = Value::makeNativeFunction("factorial", math::factorial);

    // 复数函数
    globals["arg"] = Value::makeNativeFunction("arg", math::arg);
    globals["conj"] = Value::makeNativeFunction("conj", math::conj);
    globals["re"] = Value::makeNativeFunction("re", math::re);
    globals["im"] = Value::makeNativeFunction("im", math::im);

    // 工具函数
    globals["print"] = Value::makeNativeFunction("print", utils::print);
    globals["input"] = Value::makeNativeFunction("input", utils::input);
    globals["typeof"] = Value::makeNativeFunction("typeof", utils::typeof_fn);
    globals["size"] = Value::makeNativeFunction("size", utils::size);
    globals["tostring"] = Value::makeNativeFunction("tostring", utils::tostring);
    globals["to_string"] = Value::makeNativeFunction("to_string", utils::to_string);
    globals["exit"] = Value::makeNativeFunction("exit", utils::exit);
    globals["new"] = Value::makeNativeFunction("new", utils::new_fn);
    globals["same"] = Value::makeNativeFunction("same", utils::same);
    globals["setattr"] = Value::makeNativeFunction("setattr", utils::setattr);
    globals["update"] = Value::makeNativeFunction("update", utils::update);
    globals["fraction"] = Value::makeNativeFunction("fraction", utils::fraction);
    globals["decimal"] = Value::makeNativeFunction("decimal", utils::decimal);
    globals["assert"] = Value::makeNativeFunction("assert", utils::assert_fn);

    // 类型转换函数
    globals["int"] = Value::makeNativeFunction("int", utils::to_int);
    globals["float"] = Value::makeNativeFunction("float", utils::to_float);
    globals["bool"] = Value::makeNativeFunction("bool", utils::to_bool);
    globals["string"] = Value::makeNativeFunction("string", utils::to_string_fn);
    globals["rational"] = Value::makeNativeFunction("rational", utils::to_rational);
    globals["complex"] = Value::makeNativeFunction("complex", utils::to_complex);

    // Lamina-compliant string functions
    globals["string_concat"] = Value::makeNativeFunction("string_concat", string::concat);
    globals["string_char_at"] = Value::makeNativeFunction("string_char_at", string::char_at);
    globals["string_length"] = Value::makeNativeFunction("string_length", string::length);
    globals["string_find"] = Value::makeNativeFunction("string_find", string::find);
    globals["string_sub_string"] = Value::makeNativeFunction("string_sub_string", string::sub);
    globals["string_replace_by_index"] = Value::makeNativeFunction("string_replace_by_index", string::replace_by_index);

    // 数组函数
    globals["foreach"] = Value::makeNativeFunction("foreach", array::foreach);
    globals["map"] = Value::makeNativeFunction("map", array::map);
    globals["filter"] = Value::makeNativeFunction("filter", array::filter);
    globals["reduce"] = Value::makeNativeFunction("reduce", array::reduce);
    globals["fold"] = Value::makeNativeFunction("fold", array::reduce);
    globals["push"] = Value::makeNativeFunction("push", array::push);
    globals["pop"] = Value::makeNativeFunction("pop", array::pop);
    globals["range"] = Value::makeNativeFunction("range", array::range);
    globals["concat"] = Value::makeNativeFunction("concat", array::concat);
    globals["dot"] = Value::makeNativeFunction("dot", array::dot);
    globals["norm"] = Value::makeNativeFunction("norm", array::norm);
    globals["cross"] = Value::makeNativeFunction("cross", array::cross);
    globals["det"] = Value::makeNativeFunction("det", array::det);

    // 随机命名空间
    auto random_ns = std::make_shared<std::unordered_map<std::string, Value>>();
    (*random_ns)["rand"] = Value::makeNativeFunction("random::rand", random_ns::rand);
    (*random_ns)["randint"] = Value::makeNativeFunction("random::randint", random_ns::randint);
    (*random_ns)["random"] = Value::makeNativeFunction("random::random", random_ns::random);
    globals["random"] = Value::makeModule(random_ns);

    // 时间命名空间
    globals["time"] = create_time_module();

    // CAS函数
    globals["parse"] = Value::makeNativeFunction("parse", cas::cas_parse);
    globals["differentiate"] = Value::makeNativeFunction("differentiate", cas::cas_differentiate);
    globals["solve_linear"] = Value::makeNativeFunction("solve_linear", cas::cas_solve_linear);
    globals["evaluate_at"] = Value::makeNativeFunction("evaluate_at", cas::cas_evaluate_at);
    globals["store"] = Value::makeNativeFunction("store", cas::cas_store);
    globals["load"] = Value::makeNativeFunction("load", cas::cas_load);
    globals["numerical_derivative"] = Value::makeNativeFunction("numerical_derivative", cas::cas_numerical_derivative);
    globals["integrate"] = Value::makeNativeFunction("integrate", cas::cas_integrate);
    globals["definite_integral"] = Value::makeNativeFunction("definite_integral", cas::cas_definite_integral);

    // CAS函数（带cas_前缀）
    globals["cas_parse"] = Value::makeNativeFunction("cas_parse", cas::cas_parse);
    globals["cas_differentiate"] = Value::makeNativeFunction("cas_differentiate", cas::cas_differentiate);
    globals["cas_solve_linear"] = Value::makeNativeFunction("cas_solve_linear", cas::cas_solve_linear);
    globals["cas_evaluate_at"] = Value::makeNativeFunction("cas_evaluate_at", cas::cas_evaluate_at);
    globals["cas_store"] = Value::makeNativeFunction("cas_store", cas::cas_store);
    globals["cas_load"] = Value::makeNativeFunction("cas_load", cas::cas_load);
    globals["cas_numerical_derivative"] = Value::makeNativeFunction("cas_numerical_derivative", cas::cas_numerical_derivative);

    // 字符串命名空间
    auto string_ns = std::make_shared<std::unordered_map<std::string, Value>>();
    (*string_ns)["cat"] = Value::makeNativeFunction("string::cat", string::cat);
    (*string_ns)["at"] = Value::makeNativeFunction("string::at", string::at);
    (*string_ns)["find"] = Value::makeNativeFunction("string::find", string::find);
    (*string_ns)["sub"] = Value::makeNativeFunction("string::sub", string::sub);
    (*string_ns)["length"] = Value::makeNativeFunction("string::length", string::length);
    (*string_ns)["char_at"] = Value::makeNativeFunction("string::char_at", string::char_at);
    (*string_ns)["replace_by_index"] = Value::makeNativeFunction("string::replace_by_index", string::replace_by_index);
    globals["string"] = Value::makeModule(string_ns);

    // 虚拟include模块
    globals["rumina:buffer"] = buffer::create_buffer_module();
    globals["rumina:fs"] = fs::create_fs_module();
    globals["rumina:path"] = path::create_path_module();
    globals["rumina:env"] = env::create_env_module();
    globals["rumina:process"] = process::create_process_module();
    globals["rumina:time"] = create_time_module();
    globals["rumina:stream"] = stream::create_stream_module();

    // 带命名空间前缀的字符串函数
    globals["string::cat"] = Value::makeNativeFunction("string::cat", string::cat);
    globals["string::at"] = Value::makeNativeFunction("string::at", string::at);
    globals["string::find"] = Value::makeNativeFunction("string::find", string::find);
    globals["string::sub"] = Value::makeNativeFunction("string::sub", string::sub);
    globals["string::length"] = Value::makeNativeFunction("string::length", string::length);
    globals["string::char_at"] = Value::makeNativeFunction("string::char_at", string::char_at);
    globals["string::replace_by_index"] = Value::makeNativeFunction("string::replace_by_index", string::replace_by_index);

    // 物理/化学常量
    globals["EARTH_GRAVITY"] = Value(9.80665);
    globals["MOON_GRAVITY"] = Value(1.625);
    globals["MARS_GRAVITY"] = Value(3.72076);
    globals["WATER_DENSITY"] = Value(1000.0);
    globals["STANDARD_PRESSURE"] = Value(101325.0);
    globals["STANDARD_TEMPERATURE"] = Value(273.15);
    globals["AIR_DENSITY"] = Value(1.225);
    globals["C"] = Value(2.99792458e8);
    globals["G"] = Value(6.67430e-11);
    globals["H"] = Value(6.62607015e-34);
    globals["KB"] = Value(1.380649e-23);
    globals["EPSILON_0"] = Value(8.8541878128e-12);
    globals["MU_0"] = Value(1.25663706212e-6);
    globals["AVOGADRO"] = Value(6.02214076e23);
    globals["R"] = Value(8.314462618);
    globals["FARADAY"] = Value(9.648533212e4);
    globals["AMU"] = Value(1.66053906660e-27);
    globals["MOLAR_VOLUME_IDEAL"] = Value(0.024465);
    globals["ROOM_PRESSURE"] = Value(1.0e5);
    globals["ROOM_TEMPERATURE"] = Value(297.15);
}

Value convert_to_bigint(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_bigint expects 1 argument");
    }
    const Value& val = args[0];
    switch (val.getType()) {
        case Value::Type::BigInt:
            return val;
        case Value::Type::Int:
            return Value(BigInt(static_cast<long>(val.getInt())));
        case Value::Type::Bool:
            return Value(BigInt(static_cast<long>(val.getBool() ? 1 : 0)));
        case Value::Type::String: {
            try {
                return Value(BigInt(val.getString().c_str()));
            } catch (...) {
                throw std::runtime_error("Cannot convert string to BigInt");
            }
        }
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to BigInt");
    }
}

Value convert_to_declared_type(const Value& val, DeclaredType dtype) {
    switch (dtype) {
        case DeclaredType::Int:
            return convert_to_int({val});
        case DeclaredType::Float:
            return convert_to_float({val});
        case DeclaredType::Bool:
            return convert_to_bool({val});
        case DeclaredType::String:
            return convert_to_string({val});
        case DeclaredType::Rational:
            return convert_to_rational({val});
        case DeclaredType::Irrational:
            return convert_to_irrational({val});
        case DeclaredType::Complex:
            return convert_to_complex({val});
        case DeclaredType::Array:
            return convert_to_array({val});
        case DeclaredType::BigInt:
            return convert_to_bigint({val});
        default:
            return val;
    }
}

Value convert_to_int(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_int expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Int:
            return val;
        case Value::Type::BigInt: {
            const BigInt& n = val.getBigInt();
            if (n.fits_sint_p()) {
                return Value(static_cast<int64_t>(n.get_si()));
            }
            throw std::runtime_error("BigInt too large to convert to int");
        }
        case Value::Type::Float:
            return Value(static_cast<int64_t>(val.getFloat()));
        case Value::Type::Bool:
            return Value(static_cast<int64_t>(val.getBool() ? 1 : 0));
        case Value::Type::String: {
            try {
                size_t pos;
                int64_t n = std::stoll(val.getString(), &pos);
                if (pos == val.getString().length()) {
                    return Value(n);
                }
            } catch (...) {}
            throw std::runtime_error("Cannot convert string '" + val.getString() + "' to int");
        }
        case Value::Type::Rational: {
            double f = val.getRational().get_d();
            return Value(static_cast<int64_t>(f));
        }
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to int");
    }
}

Value convert_to_float(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_float expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Float:
            return val;
        case Value::Type::Int:
            return Value(static_cast<double>(val.getInt()));
        case Value::Type::BigInt: {
            double f = val.getBigInt().get_d();
            if (std::isfinite(f)) {
                return Value(f);
            }
            throw std::runtime_error("BigInt too large to convert to float");
        }
        case Value::Type::Bool:
            return Value(val.getBool() ? 1.0 : 0.0);
        case Value::Type::String: {
            try {
                size_t pos;
                double f = std::stod(val.getString(), &pos);
                if (pos == val.getString().length()) {
                    return Value(f);
                }
            } catch (...) {}
            throw std::runtime_error("Cannot convert string '" + val.getString() + "' to float");
        }
        case Value::Type::Rational:
            return Value(val.getRational().get_d());
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to float");
    }
}

Value convert_to_bool(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_bool expects 1 argument");
    }
    
    return Value(args[0].isTruthy());
}

Value convert_to_string(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_string expects 1 argument");
    }
    
    return Value(args[0].toString());
}

Value convert_to_rational(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_rational expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Rational:
            return val;
        case Value::Type::Int:
            return Value(BigRational(static_cast<long>(val.getInt()), 1));
        case Value::Type::Bool:
            return Value(BigRational(val.getBool() ? 1 : 0, 1));
        case Value::Type::Float: {
            double f = val.getFloat();
            long long num = static_cast<long long>(f * 1e12);
            long long den = 1000000000000LL;
            long long gcd_val = std::gcd(static_cast<long long>(std::abs(num)), static_cast<long long>(den));
            num /= gcd_val;
            den /= gcd_val;
            return Value(BigRational(static_cast<long>(num), static_cast<long>(den)));
        }
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to rational");
    }
}

Value convert_to_irrational(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_irrational expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Irrational:
            return val;
        case Value::Type::Int: {
            int64_t n = val.getInt();
            return Value(IrrationalValue::makeSqrt(
                std::make_shared<Value>(Value(n * n))));
        }
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to irrational");
    }
}

Value convert_to_complex(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_complex expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Complex:
            return val;
        case Value::Type::Int:
            return Value(
                std::make_shared<Value>(val),
                std::make_shared<Value>(Value(static_cast<int64_t>(0))));
        case Value::Type::Float:
            return Value(
                std::make_shared<Value>(val),
                std::make_shared<Value>(Value(0.0)));
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to complex");
    }
}

Value convert_to_array(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("convert_to_array expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Array:
            return val;
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to array");
    }
}

} // namespace builtin
} // namespace rumina
