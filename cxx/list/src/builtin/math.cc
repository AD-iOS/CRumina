#include <builtin/math.h>

#include <cmath>

namespace rumina {
namespace builtin {
namespace math {

Value sqrt(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("sqrt expects 1 argument");
    }
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Int) {
        int64_t n = val.getInt();
        if (n < 0) {
            int64_t abs_n = std::abs(n);
            double sqrt_val = std::sqrt(static_cast<double>(abs_n));
            
            if (sqrt_val == std::floor(sqrt_val)) {
                return Value(
                    std::make_shared<Value>(Value(static_cast<int64_t>(0))), // std::make_shared<Value>(Value(0)),
                    std::make_shared<Value>(Value(static_cast<int64_t>(sqrt_val))));
            } else {
                return Value(
                    std::make_shared<Value>(Value(static_cast<int64_t>(0))), // std::make_shared<Value>(Value(0)),
                    std::make_shared<Value>(Value(IrrationalValue::makeSqrt(
                        std::make_shared<Value>(Value(abs_n))))));
            }
        } else {
            double sqrt_val = std::sqrt(static_cast<double>(n));
            if (sqrt_val == std::floor(sqrt_val)) {
                return Value(static_cast<int64_t>(sqrt_val));
            } else {
                return Value(IrrationalValue::makeSqrt(
                    std::make_shared<Value>(Value(n))));
            }
        }
    } else if (val.getType() == Value::Type::Float) {
        double f = val.getFloat();
        if (f < 0) {
            double abs_f = std::abs(f);
            return Value(
                std::make_shared<Value>(Value(0.0)),
                std::make_shared<Value>(Value(std::sqrt(abs_f))));
        } else {
            return Value(std::sqrt(f));
        }
    } else if (val.getType() == Value::Type::Irrational) {
        return Value(IrrationalValue::makeSqrt(
            std::make_shared<Value>(val)));
    }
    
    throw std::runtime_error("sqrt expects number, got " + val.typeName());
}

Value pi(const std::vector<Value>& args) {
    if (args.size() != 0) {
        throw std::runtime_error("pi expects 0 arguments");
    }
    return Value(IrrationalValue::makePi());
}

Value e(const std::vector<Value>& args) {
    if (args.size() != 0) {
        throw std::runtime_error("e expects 0 arguments");
    }
    return Value(IrrationalValue::makeE());
}

Value sin(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("sin expects 1 argument");
    }
    
    double val = args[0].toFloat();
    return Value(std::sin(val));
}

Value cos(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("cos expects 1 argument");
    }
    
    double val = args[0].toFloat();
    return Value(std::cos(val));
}

Value tan(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("tan expects 1 argument");
    }
    
    double val = args[0].toFloat();
    return Value(std::tan(val));
}

Value exp(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("exp expects 1 argument");
    }
    
    double val = args[0].toFloat();
    return Value(std::exp(val));
}

Value abs_fn(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("abs expects 1 argument");
    }
    
    const Value& val = args[0];
    
    switch (val.getType()) {
        case Value::Type::Int:
            return Value(std::llabs(val.getInt()));
        case Value::Type::Float:
            return Value(std::fabs(val.getFloat()));
        case Value::Type::Irrational: {
            double f = irrationalToFloat(val.getIrrational());
            if (f < 0) {
                return Value(-f);
            } else {
                return val;
            }
        }
        case Value::Type::Complex: {
            auto [re, im] = val.getComplex();
            double re_val = re->toFloat();
            double im_val = im->toFloat();
            double magnitude = std::sqrt(re_val * re_val + im_val * im_val);
            
            if (magnitude == std::floor(magnitude)) {
                return Value(static_cast<int64_t>(magnitude));
            } else {
                return Value(magnitude);
            }
        }
        default:
            throw std::runtime_error("abs expects number, got " + val.typeName());
    }
}

Value arg(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("arg expects 1 argument");
    }
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Complex) {
        auto [re, im] = val.getComplex();
        double re_val = re->toFloat();
        double im_val = im->toFloat();
        return Value(std::atan2(im_val, re_val));
    } else if (val.getType() == Value::Type::Int) {
        int64_t n = val.getInt();
        if (n < 0) {
            return Value(IrrationalValue::makePi());
        } else {
            return Value(static_cast<int64_t>(0)); // return Value(0);
        }
    } else if (val.getType() == Value::Type::Float) {
        double f = val.getFloat();
        if (f < 0) {
            return Value(IrrationalValue::makePi());
        } else {
            return Value(static_cast<int64_t>(0)); // return Value(0);
        }
    }
    
    throw std::runtime_error("arg expects complex or real number, got " + val.typeName());
}

Value conj(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("conj expects 1 argument");
    }
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Complex) {
        auto [re, im] = val.getComplex();
        
        if (im->getType() == Value::Type::Int) {
            return Value(re, std::make_shared<Value>(Value(-im->getInt())));
        } else if (im->getType() == Value::Type::Float) {
            return Value(re, std::make_shared<Value>(Value(-im->getFloat())));
        } else if (im->getType() == Value::Type::Rational) {
            return Value(re, std::make_shared<Value>(Value(-im->getRational())));
        } else {
            double im_val = im->toFloat();
            return Value(re, std::make_shared<Value>(Value(-im_val)));
        }
    }
    
    return val;
}

Value re(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("re expects 1 argument");
    }
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Complex) {
        auto [re, _] = val.getComplex();
        return *re;
    }
    
    return val;
}

Value im(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("im expects 1 argument");
    }
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Complex) {
        auto [_, im] = val.getComplex();
        return *im;
    }
    
    return Value(static_cast<int64_t>(0)); // return Value(0);
}

Value log(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("log expects 1 argument");
    }
    
    double val = args[0].toFloat();
    if (val <= 0) {
        throw std::runtime_error("log domain error: input must be positive");
    }
    return Value(std::log10(val));
}

Value ln(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("ln expects 1 argument");
    }
    
    double val = args[0].toFloat();
    if (val <= 0) {
        throw std::runtime_error("ln domain error: input must be positive");
    }
    return Value(std::log(val));
}

Value logbase(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("logBASE expects 2 arguments (base, value)");
    }
    
    double base = args[0].toFloat();
    double val = args[1].toFloat();
    
    if (base <= 0 || std::abs(base - 1.0) < 1e-10) {
        throw std::runtime_error("logBASE domain error: base must be positive and not 1");
    }
    if (val <= 0) {
        throw std::runtime_error("logBASE domain error: value must be positive");
    }
    
    return Value(std::log(val) / std::log(base));
}

Value factorial(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("factorial expects 1 argument");
    }
    
    const Value& val = args[0];
    
    if (val.getType() == Value::Type::Int) {
        int64_t n = val.getInt();
        if (n < 0) {
            throw std::runtime_error("Factorial of negative number");
        }
        
        BigInt result = 1;
        for (int64_t i = 2; i <= n; ++i) {
            result = result * BigInt(static_cast<long>(i));
        }
        return Value(result);
    } else if (val.getType() == Value::Type::BigInt) {
        const BigInt& n = val.getBigInt();
        if (n < 0) {
            throw std::runtime_error("Factorial of negative number");
        }
        
        BigInt result = 1;
        BigInt i = 2;
        while (i <= n) {
            result = result * i;
            i += 1;
        }
        return Value(result);
    }
    
    throw std::runtime_error("factorial expects integer, got " + val.typeName());
}

} // namespace math
} // namespace builtin
} // namespace rumina
