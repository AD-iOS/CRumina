#include <value_ops.h>
#include <interpreter.h>

#include <cmath>
#include <thread>
#include <vector>
#include <algorithm>

namespace rumina {

// 大数幂运算优化
BigInt bigint_pow_optimized(const BigInt& base, uint32_t exponent) {
    if (exponent < PARALLEL_POW_THRESHOLD) {
        BigInt result = 1;
        BigInt b = base;
        uint32_t exp = exponent;
        
        while (exp > 0) {
            if (exp & 1) {
                result *= b;
            }
            b *= b;
            exp >>= 1;
        }
        return result;
    }
    
    return bigint_pow_parallel(base, exponent);
}

// 并行大数幂运算
BigInt bigint_pow_parallel(const BigInt& base, uint32_t exponent) {
    if (exponent == 0) return 1;
    if (exponent == 1) return base;
    
    if (exponent < 500) {
        BigInt result = 1;
        BigInt b = base;
        uint32_t exp = exponent;
        while (exp > 0) {
            if (exp & 1) result *= b;
            b *= b;
            exp >>= 1;
        }
        return result;
    }
    
    int num_chunks;
    if (exponent >= 100000) {
        num_chunks = 8;
    } else if (exponent >= 10000) {
        num_chunks = 4;
    } else {
        num_chunks = 2;
    }
    
    uint32_t chunk_size = exponent / num_chunks;
    uint32_t remainder = exponent % num_chunks;
    
    if (chunk_size < 100) {
        BigInt result = 1;
        BigInt b = base;
        uint32_t exp = exponent;
        while (exp > 0) {
            if (exp & 1) result *= b;
            b *= b;
            exp >>= 1;
        }
        return result;
    }
    
    std::vector<BigInt> chunk_results(num_chunks);
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_chunks; ++i) {
        threads.emplace_back([&base, chunk_size, &chunk_results, i]() {
            chunk_results[i] = bigint_pow_parallel(base, chunk_size);
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    BigInt result = 1;
    for (const auto& cr : chunk_results) {
        result *= cr;
    }
    
    if (remainder > 0) {
        BigInt remainder_result = bigint_pow_parallel(base, remainder);
        result *= remainder_result;
    }
    
    return result;
}

// 幂运算符号处理
Result<Value> compute_power(double base, double exponent) {
    double denom_approx = std::round(1.0 / exponent);
    bool is_simple_root = std::abs(1.0 / denom_approx - exponent) < 1e-10;
    
    if (is_simple_root && denom_approx > 0) {
        uint32_t n = static_cast<uint32_t>(denom_approx);
        
        if (base < 0) {
            bool is_odd_root = (n % 2 == 1);
            
            if (is_odd_root) {
                double abs_base = std::abs(base);
                double root_val = std::pow(abs_base, exponent);
                
                if (std::abs(std::round(root_val) - root_val) < 1e-10) {
                    return Ok(Value(static_cast<int64_t>(-std::round(root_val))));
                }
                
                std::shared_ptr<Value> abs_val = std::make_shared<Value>(Value(static_cast<int64_t>(abs_base)));
                IrrationalValue root;
                if (n == 2) {
                    root = IrrationalValue::makeSqrt(abs_val);
                } else {
                    root = IrrationalValue::makeRoot(n, abs_val);
                }
                
                return Ok(Value(IrrationalValue::makeProduct(
                    std::make_shared<Value>(Value(static_cast<int64_t>(-1))),
                    std::make_shared<IrrationalValue>(std::move(root)))));
            } else {
                double abs_base = std::abs(base);
                double root_val = std::pow(abs_base, exponent);
                
                if (std::abs(std::round(root_val) - root_val) < 1e-10) {
                    return Ok(Value(
                        std::make_shared<Value>(Value(static_cast<int64_t>(0))),
                        std::make_shared<Value>(Value(static_cast<int64_t>(std::round(root_val))))));
                }
                
                std::shared_ptr<Value> abs_val = std::make_shared<Value>(Value(static_cast<int64_t>(abs_base)));
                IrrationalValue root;
                if (n == 2) {
                    root = IrrationalValue::makeSqrt(abs_val);
                } else {
                    root = IrrationalValue::makeRoot(n, abs_val);
                }
                
                return Ok(Value(
                    std::make_shared<Value>(Value(static_cast<int64_t>(0))),
                    std::make_shared<Value>(Value(std::move(root)))));
            }
        } else {
            double root_val = std::pow(base, exponent);
            if (std::abs(std::round(root_val) - root_val) < 1e-10) {
                return Ok(Value(static_cast<int64_t>(std::round(root_val))));
            }
            
            if (n == 2) {
                return Ok(Value(IrrationalValue::makeSqrt(
                    std::make_shared<Value>(Value(static_cast<int64_t>(base))))));
            } else {
                return Ok(Value(IrrationalValue::makeRoot(n,
                    std::make_shared<Value>(Value(static_cast<int64_t>(base))))));
            }
        }
    }
    
    double result = std::pow(base, exponent);
    
    if (std::abs(std::round(result) - result) < 1e-10) {
        return Ok(Value(static_cast<int64_t>(std::round(result))));
    } else {
        return Ok(Value(result));
    }
}

// 二进制操作
Result<Value> value_binary_op(const Value& left, BinOp op, const Value& right) {
    try {
        // 整数快速路径
        if (left.getType() == Value::Type::Int && right.getType() == Value::Type::Int) {
            int64_t a = left.getInt();
            int64_t b = right.getInt();
            
            switch (op) {
                case BinOp::Add:
                    return Ok(Value(static_cast<int64_t>(a + b)));
                case BinOp::Sub:
                    return Ok(Value(static_cast<int64_t>(a - b)));
                case BinOp::Mul:
                    return Ok(Value(static_cast<int64_t>(a * b)));
                case BinOp::Div:
                    if (b == 0) {
                        return Err<Value>("Division by zero");
                    }
                    return Ok(Value(BigRational(static_cast<long>(a), static_cast<long>(b))));
                case BinOp::Mod:
                    return Ok(Value(static_cast<int64_t>(a % b)));
                case BinOp::Pow: {
                    if (b < 0) {
                        double result = std::pow(static_cast<double>(a), static_cast<double>(b));
                        return Ok(Value(result));
                    } else {
                        if (a == 0) return Ok(Value(static_cast<int64_t>(0)));
                        if (a == 1) return Ok(Value(static_cast<int64_t>(1)));
                        
                        double log_result = std::log2(std::abs(static_cast<double>(a))) * b;
                        if (log_result > 62) {
                            BigInt a_big = BigInt(static_cast<long>(a));
                            return Ok(Value(bigint_pow_optimized(a_big, static_cast<uint32_t>(b))));
                        } else {
                            int64_t result = 1;
                            for (int32_t i = 0; i < b; ++i) {
                                result *= a;
                            }
                            return Ok(Value(static_cast<int64_t>(result)));
                        }
                    }
                }
                case BinOp::Equal:
                    return Ok(Value(a == b));
                case BinOp::NotEqual:
                    return Ok(Value(a != b));
                case BinOp::Greater:
                    return Ok(Value(a > b));
                case BinOp::GreaterEq:
                    return Ok(Value(a >= b));
                case BinOp::Less:
                    return Ok(Value(a < b));
                case BinOp::LessEq:
                    return Ok(Value(a <= b));
                default:
                    return Err<Value>("Unsupported operation: int " + std::to_string(static_cast<int>(op)) + " int");
            }
        }
        
        // BigInt 快速路径
        if (left.getType() == Value::Type::BigInt && right.getType() == Value::Type::BigInt) {
            const BigInt& a = left.getBigInt();
            const BigInt& b = right.getBigInt();
            switch (op) {
                case BinOp::Add:
                    return Ok(Value(static_cast<const BigInt&>(a + b)));
                case BinOp::Sub:
                    return Ok(Value(static_cast<const BigInt&>(a - b)));
                case BinOp::Mul:
                    return Ok(Value(static_cast<const BigInt&>(a * b)));
            /*
                case BinOp::Add:
                    return Ok(Value(a + b));
                case BinOp::Sub:
                    return Ok(Value(a - b));
                case BinOp::Mul:
                    return Ok(Value(a * b));
            */
                case BinOp::Div:
                    if (b == 0) {
                        return Err<Value>("Division by zero");
                    }
                    return Ok(Value(BigRational(a, b)));
                case BinOp::Mod:
                    if (b == 0) {
                        return Err<Value>("Division by zero");
                    }
                    return Ok(Value(static_cast<const BigInt&>(a % b)));
            /*
                case BinOp::Mod:
                    if (b == 0) {
                        return Err<Value>("Division by zero");
                    }
                    return Ok(Value(a % b));
            */
                case BinOp::Pow: {
                    if (b.fits_uint_p()) {
                        unsigned long exp = b.get_ui();
                        if (exp <= std::numeric_limits<uint32_t>::max()) {
                            return Ok(Value(bigint_pow_optimized(a, static_cast<uint32_t>(exp))));
                        }
                    }
                    double a_float = a.get_d();
                    double b_float = b.get_d();
                    return Ok(Value(std::pow(a_float, b_float)));
                }
                case BinOp::Equal:
                    return Ok(Value(a == b));
                case BinOp::NotEqual:
                    return Ok(Value(a != b));
                case BinOp::Greater:
                    return Ok(Value(a > b));
                case BinOp::GreaterEq:
                    return Ok(Value(a >= b));
                case BinOp::Less:
                    return Ok(Value(a < b));
                case BinOp::LessEq:
                    return Ok(Value(a <= b));
                default:
                    return Err<Value>("Unsupported operation: bigint " + std::to_string(static_cast<int>(op)) + " bigint");
            }
        }
        
        // 布尔运算快速路径
        if (left.getType() == Value::Type::Bool && right.getType() == Value::Type::Bool) {
            bool a = left.getBool();
            bool b = right.getBool();
            
            switch (op) {
                case BinOp::And:
                    return Ok(Value(a && b));
                case BinOp::Or:
                    return Ok(Value(a || b));
                case BinOp::Equal:
                    return Ok(Value(a == b));
                case BinOp::NotEqual:
                    return Ok(Value(a != b));
                default:
                    return Err<Value>("Unsupported operation: bool " + std::to_string(static_cast<int>(op)) + " bool");
            }
        }
        
        // Null 比较
        if (left.getType() == Value::Type::Null && right.getType() == Value::Type::Null) {
            switch (op) {
                case BinOp::Equal:
                    return Ok(Value(true));
                case BinOp::NotEqual:
                    return Ok(Value(false));
                default:
                    return Err<Value>("Unsupported operation: null " + std::to_string(static_cast<int>(op)) + " null");
            }
        }

        // 字符串與任何類型的連接
        if (op == BinOp::Add && (left.getType() == Value::Type::String || right.getType() == Value::Type::String)) {
            // 只要有一方是字符串,就將兩邊都轉換為字符串並連接
            std::string result = left.toString() + right.toString();
            return Ok(Value(result));
        }

        // 字符串操作
        if (left.getType() == Value::Type::String && right.getType() == Value::Type::String) {
            const std::string& a = left.getString();
            const std::string& b = right.getString();
            switch (op) {
                case BinOp::Add:
                    return Ok(Value(a + b));
                case BinOp::Equal:
                    return Ok(Value(a == b));
                case BinOp::NotEqual:
                    return Ok(Value(a != b));
                case BinOp::Greater:
                    return Ok(Value(a > b));
                case BinOp::GreaterEq:
                    return Ok(Value(a >= b));
                case BinOp::Less:
                    return Ok(Value(a < b));
                case BinOp::LessEq:
                    return Ok(Value(a <= b));
                default:
                    return Err<Value>("Unsupported operation: string " + 
                                       std::to_string(static_cast<int>(op)) + " string");
    }
}
        
        // Null 与非 Null 比较
        if (left.getType() == Value::Type::Null || right.getType() == Value::Type::Null) {
            switch (op) {
                case BinOp::Equal:
                    return Ok(Value(false));
                case BinOp::NotEqual:
                    return Ok(Value(true));
                default:
                    return Err<Value>("Unsupported operation: " + left.typeName() + 
                               " " + std::to_string(static_cast<int>(op)) + " " + right.typeName());
            }
        }
/*
        Interpreter interpreter;
        auto result = interpreter.eval_binary_op(left, op, right);
        if (result.is_error()) return Err<Value>(result.error());
        return Ok(result.value());
*/
        return Err<Value>("Unsupported operation: " + left.typeName() + 
                          " " + std::to_string(static_cast<int>(op)) + " " + right.typeName());
    } catch (const std::exception& e) {
        return Err<Value>(e.what());
    }
}

// 一元操作
Result<Value> value_unary_op(UnaryOp op, const Value& val) {
    try {
        switch (op) {
            case UnaryOp::Neg: {
                switch (val.getType()) {
                    case Value::Type::Int:
                        return Ok(Value(static_cast<int64_t>(-val.getInt())));
                    case Value::Type::Float:
                        return Ok(Value(-val.getFloat()));
                    case Value::Type::BigInt:
                        // return Ok(Value(-val.getBigInt()));
                        return Ok(Value(static_cast<const BigInt&>(-val.getBigInt())));
                    case Value::Type::Rational:
                        return Ok(Value(-val.getRational()));
                    default: {
                        // Interpreter interpreter;
                        // auto result = interpreter.eval_unary_op(op, val);
                        // if (result.is_error()) return Err<Value>(result.error());
                        // return Ok(result.value());
                        return Err<Value>("Cannot negate " + val.typeName());
                    }
                }
            }
            case UnaryOp::Not: {
                if (val.getType() == Value::Type::Bool) {
                    return Ok(Value(!val.getBool()));
                }
                return Err<Value>("Cannot apply 'not' to " + val.typeName());
            }
            case UnaryOp::Factorial: {
                Interpreter interpreter;
                auto result = interpreter.eval_unary_op(op, val);
                if (result.is_error()) return Err<Value>(result.error());
                return Ok(result.value());
            }
        }
        return Err<Value>("Unknown unary operation");
    } catch (const std::exception& e) {
        return Err<Value>(e.what());
    }
}

// 无理数乘法
Result<Value> multiply_irrationals(const IrrationalValue& a, const IrrationalValue& b) {
    try {
        if (a.getKind() == IrrationalValue::Kind::Sqrt && 
            b.getKind() == IrrationalValue::Kind::Sqrt) {
            
            auto product = value_binary_op(*a.getSqrtValue(), BinOp::Mul, *b.getSqrtValue());
            if (product.is_error()) return Err<Value>(product.error());
            
            if (product.value().getType() == Value::Type::Int) {
                int64_t n = product.value().getInt();
                if (n >= 0) {
                    double sqrt_val = std::sqrt(static_cast<double>(n));
                    if (sqrt_val == std::floor(sqrt_val)) {
                        return Ok(Value(static_cast<int64_t>(sqrt_val)));
                    }
                }
            }
            
            return Ok(Value(IrrationalValue::makeSqrt(
                std::make_shared<Value>(product.value()))));
        }
        
        Interpreter interpreter;
        auto result = interpreter.eval_binary_op(Value(a), BinOp::Mul, Value(b));
        if (result.is_error()) return Err<Value>(result.error());
        return Ok(result.value());
    } catch (const std::exception& e) {
        return Err<Value>(e.what());
    }
}

} // namespace rumina
