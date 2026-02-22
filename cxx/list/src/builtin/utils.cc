#include <builtin/utils.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <numeric>

namespace rumina {
namespace builtin {
namespace utils {

static std::optional<std::string> float_to_rational(double f) {
    if (!std::isfinite(f)) {
        return std::nullopt;
    }
    
    const double precision = 1e-10;
    int64_t h1 = 1, h2 = 0;
    int64_t k1 = 0, k2 = 1;
    double b = f;
    
    for (int i = 0; i < 100; ++i) {
        int64_t a = static_cast<int64_t>(std::floor(b));
        int64_t aux = h1;
        h1 = a * h1 + h2;
        h2 = aux;
        aux = k1;
        k1 = a * k1 + k2;
        k2 = aux;
        
        if (std::abs(f - static_cast<double>(h1) / k1) < precision) {
            return std::to_string(h1) + "/" + std::to_string(k1);
        }
        
        b = 1.0 / (b - a);
        if (!std::isfinite(b)) break;
    }
    
    return std::nullopt;
}

Value print(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        
        if (args[i].getType() == Value::Type::Float) {
            auto rational = float_to_rational(args[i].getFloat());
            if (rational.has_value()) {
                std::cout << rational.value();
            } else {
                std::cout << args[i].toString();
            }
        } else {
            std::cout << args[i].toString();
        }
    }
    std::cout << std::endl;
    
    return Value();
}

Value input(const std::vector<Value>& args) {
    if (!args.empty()) {
        std::cout << args[0].toString();
        std::cout.flush();
    }
    
    std::string input;
    std::getline(std::cin, input);
    
    size_t start = input.find_first_not_of(" \t\r\n");
    size_t end = input.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return Value();
    }
    input = input.substr(start, end - start + 1);
    
    try {
        size_t pos;
        int64_t n = std::stoll(input, &pos);
        if (pos == input.length()) {
            return Value(n);
        }
    } catch (...) {
    }
    
    try {
        size_t pos;
        double f = std::stod(input, &pos);
        if (pos == input.length()) {
            return Value(f);
        }
    } catch (...) {
    }
    
    return Value(input);
}

Value typeof_fn(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("typeof expects 1 argument");
    }
    return Value(args[0].typeName());
}

Value size(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("size expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Array:
            return Value(static_cast<int64_t>(val.getArray()->size()));
        case Value::Type::Struct:
            return Value(static_cast<int64_t>(val.getStruct()->size()));
        case Value::Type::String:
            return Value(static_cast<int64_t>(val.getString().length()));
        default:
            throw std::runtime_error("size expects array/struct/string, got " + val.typeName());
    }
}

Value tostring(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("tostring expects 1 argument");
    }
    return Value(args[0].toString());
}

Value to_string(const std::vector<Value>& args) {
    return tostring(args);
}

Value exit(const std::vector<Value>& args) {
    int64_t code = 0;
    if (!args.empty()) {
        code = args[0].toInt();
    }
    std::exit(static_cast<int>(code));
}

Value new_fn(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("new expects 1 argument (struct)");
    }
    
    const Value& val = args[0];
    
    if (val.getType() != Value::Type::Struct) {
        throw std::runtime_error("new expects struct, got " + val.typeName());
    }
    
    auto original = val.getStruct();
    auto new_struct = std::make_shared<std::unordered_map<std::string, Value>>(*original);
    
    (*new_struct)["__parent__"] = val;
    
    return Value::makeStruct(new_struct);
}

Value same(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("same expects 2 arguments");
    }
    
    bool result;
    
    if (args[0].getType() == Value::Type::Struct && 
        args[1].getType() == Value::Type::Struct) {
        result = (args[0].getStruct() == args[1].getStruct());
    } else {
        result = (args[0] == args[1]);
    }
    
    return Value(result);
}

Value setattr(const std::vector<Value>& args) {
    if (args.size() != 3) {
        throw std::runtime_error("setattr expects 3 arguments (object, key, value)");
    }
    
    const Value& obj = args[0];
    const Value& key = args[1];
    const Value& value = args[2];
    
    if (obj.getType() != Value::Type::Struct) {
        throw std::runtime_error("setattr expects struct, got " + obj.typeName());
    }
    
    if (key.getType() != Value::Type::String) {
        throw std::runtime_error("setattr expects string key");
    }
    
    (*obj.getStruct())[key.getString()] = value;
    return Value();
}

Value update(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("update expects 2 arguments (target, source)");
    }
    
    const Value& target = args[0];
    const Value& source = args[1];
    
    if (target.getType() != Value::Type::Struct || 
        source.getType() != Value::Type::Struct) {
        throw std::runtime_error("update expects two structs");
    }
    
    auto target_map = target.getStruct();
    auto source_map = source.getStruct();
    
    for (const auto& [key, value] : *source_map) {
        (*target_map)[key] = value;
    }
    
    return Value();
}

Value fraction(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fraction expects 1 argument");
    }
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Float) {
        double f = val.getFloat();
        if (!std::isfinite(f)) {
            throw std::runtime_error("Cannot convert infinite or NaN to fraction");
        }
        
        const double precision = 1e-10;
        int64_t h1 = 1, h2 = 0;
        int64_t k1 = 0, k2 = 1;
        double b = f;
        
        for (int i = 0; i < 100; ++i) {
            int64_t a = static_cast<int64_t>(std::floor(b));
            int64_t aux = h1;
            h1 = a * h1 + h2;
            h2 = aux;
            aux = k1;
            k1 = a * k1 + k2;
            k2 = aux;
            
            if (std::abs(f - static_cast<double>(h1) / k1) < precision) {
                return Value(BigRational(static_cast<long>(h1), static_cast<long>(k1)));
            }
            
            b = 1.0 / (b - a);
            if (!std::isfinite(b)) break;
        }
        
        return Value(BigRational(
            static_cast<long>(h1), 
            static_cast<long>(k1)
        ));
        // return Value(BigRational::from_float(f).value_or(BigRational(0, 1)));
    } else if (val.getType() == Value::Type::Int) {
        return Value(BigRational(static_cast<long>(val.getInt()), 1));
    } else if (val.getType() == Value::Type::Rational) {
        return val;
    }
    
    throw std::runtime_error("Cannot convert " + val.typeName() + " to fraction");
}

Value decimal(const std::vector<Value>& args) {
    if (args.empty() || args.size() > 2) {
        throw std::runtime_error("decimal expects 1 or 2 arguments");
    }
    
    int32_t precision = -1;
    if (args.size() == 2) {
        if (args[1].getType() != Value::Type::Int) {
            throw std::runtime_error("decimal precision must be an integer");
        }
        int64_t p = args[1].getInt();
        if (p < 0 || p > 15) {
            throw std::runtime_error("decimal precision must be a non-negative integer <= 15");
        }
        precision = static_cast<int32_t>(p);
    }
    
    auto apply_precision = [precision](double f) -> Value {
        if (precision >= 0) {
            double factor = std::pow(10.0, precision);
            return Value(std::round(f * factor) / factor);
        } else {
            return Value(f);
        }
    };
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Rational) {
        const BigRational& r = val.getRational();
        double f = r.get_d();
        return apply_precision(f);
    } else if (val.getType() == Value::Type::Int) {
        return apply_precision(static_cast<double>(val.getInt()));
    } else if (val.getType() == Value::Type::Float) {
        return apply_precision(val.getFloat());
    } else if (val.getType() == Value::Type::Complex) {
        auto [re, im] = val.getComplex();
        double re_float = re->toFloat();
        double im_float = im->toFloat();
        
        std::ostringstream oss;
        if (im_float >= 0) {
            oss << re_float << "+" << im_float << "i";
        } else {
            oss << re_float << im_float << "i";
        }
        return Value(oss.str());
    }
    
    throw std::runtime_error("Cannot convert " + val.typeName() + " to decimal");
}

Value assert_fn(const std::vector<Value>& args) {
    if (args.empty()) {
        throw std::runtime_error("assert expects at least 1 argument");
    }
    
    if (args.size() > 2) {
        throw std::runtime_error("assert expects at most 2 arguments");
    }
    
    const Value& condition = args[0];
    
    if (!condition.isTruthy()) {
        std::string message;
        if (args.size() == 2) {
            if (args[1].getType() == Value::Type::String) {
                message = args[1].getString();
            } else {
                message = "Assertion failed: " + args[1].toString();
            }
        } else {
            message = "Assertion failed";
        }
        
        throw std::runtime_error(message);
    }
    
    return Value();
}

Value to_int(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("int expects 1 argument");
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
            return Value(static_cast<int64_t>(val.getBool() ? 1 : 0)); // return Value(val.getBool() ? 1 : 0);
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

Value to_float(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("float expects 1 argument");
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

Value to_bool(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("bool expects 1 argument");
    }
    
    return Value(args[0].isTruthy());
}

Value to_string_fn(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("string expects 1 argument");
    }
    
    return Value(args[0].toString());
}

Value to_rational(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("rational expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Rational:
            return val;
        case Value::Type::Int:
            return Value(BigRational(static_cast<long>(val.getInt()), 1));
        case Value::Type::Float: {
            double f = val.getFloat();
            
            if (std::isnan(f)) {
                throw std::runtime_error("Cannot convert NaN to rational");
            }
            if (std::isinf(f)) {
                throw std::runtime_error("Cannot convert infinite value to rational");
            }
            
            const double EPSILON = 1e-10;
            const int MAX_ITERATIONS = 100;
            
            long long h1 = 1, h2 = 0;
            long long k1 = 0, k2 = 1;
            double b = f;
            
            for (int i = 0; i < MAX_ITERATIONS; ++i) {
                long long a = static_cast<long long>(std::floor(b));
                
                long long hp = a * h1 + h2;
                long long kp = a * k1 + k2;
                
                if (hp > 1000000000LL || kp > 1000000000LL) {
                    break;
                }
                
                double approx = static_cast<double>(hp) / static_cast<double>(kp);
                if (std::abs(f - approx) < EPSILON) {
                    return Value(BigRational(static_cast<long>(hp), static_cast<long>(kp)));
                }
                
                h2 = h1; h1 = hp;
                k2 = k1; k1 = kp;
                
                b = 1.0 / (b - a);
                if (!std::isfinite(b)) break;
            }

            double scaled = f;
            long long denom = 1;
            
            for (int precision = 1; precision <= 12; ++precision) {
                scaled *= 10.0;
                denom *= 10;
                
                double rounded = std::round(scaled);
                if (std::abs(scaled - rounded) < EPSILON) {
                    long long num = static_cast<long long>(rounded);
                    long long gcd_val = std::gcd(static_cast<long long>(std::abs(num)), static_cast<long long>(denom));
                    num /= gcd_val;
                    denom /= gcd_val;
                    
                    if (num <= 1000000000LL && denom <= 1000000000LL) {
                        return Value(BigRational(static_cast<long>(num), static_cast<long>(denom)));
                    }
                }
            }
            
            long long num = static_cast<long long>(f * 1e12);
            long long den = 1000000000000LL;
            
            long long gcd_val = std::gcd(static_cast<long long>(std::abs(num)), static_cast<long long>(den));
            num /= gcd_val;
            den /= gcd_val;
            
            if (num > 1000000000LL) num = 1000000000LL;
            if (den > 1000000000LL) den = 1000000000LL;
            
            return Value(BigRational(static_cast<long>(num), static_cast<long>(den)));
        }
        case Value::Type::Bool:
            return Value(BigRational(val.getBool() ? 1 : 0, 1));
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to rational");
    }
}

/*

Value to_rational(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("rational expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Rational:
            return val;
        case Value::Type::Int:
            return Value(BigRational(static_cast<long>(val.getInt()), 1));
        case Value::Type::Float: {
            // auto r = BigRational::from_float(val.getFloat());

            if (r.has_value()) {
                return Value(r.value());
            }
            return Value(BigRational(0, 1));
        }
        case Value::Type::Bool:
            return Value(BigRational(val.getBool() ? 1 : 0, 1));
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to rational");
    }
}
*/

Value to_complex(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("complex expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Complex:
            return val;
        case Value::Type::Int:
            return Value(
                std::make_shared<Value>(val),
                std::make_shared<Value>(Value(static_cast<int64_t>(0))) // std::make_shared<Value>(Value(0)));
            );
        case Value::Type::Float:
            return Value(
                std::make_shared<Value>(val),
                std::make_shared<Value>(Value(0.0)));
        default:
            throw std::runtime_error("Cannot convert " + val.typeName() + " to complex");
    }
}

} // namespace utils
} // namespace builtin
} // namespace rumina
