#include <value.h>
#include <ast.h>

#include <sstream>
#include <iomanip>
#include <cmath>

namespace rumina {

// IrrationalValue 静态工厂方法实现 - 只保留一份
IrrationalValue IrrationalValue::makeSqrt(std::shared_ptr<Value> value) {
    IrrationalValue irr;
    irr.kind_ = Kind::Sqrt;
    irr.sqrt_value_ = value;
    return irr;
}

IrrationalValue IrrationalValue::makeRoot(uint32_t degree, std::shared_ptr<Value> value) {
    IrrationalValue irr;
    irr.kind_ = Kind::Root;
    irr.root_degree_ = degree;
    irr.root_value_ = value;
    return irr;
}

IrrationalValue IrrationalValue::makePi() {
    IrrationalValue irr;
    irr.kind_ = Kind::Pi;
    return irr;
}

IrrationalValue IrrationalValue::makeE() {
    IrrationalValue irr;
    irr.kind_ = Kind::E;
    return irr;
}

IrrationalValue IrrationalValue::makeProduct(std::shared_ptr<Value> coeff, 
                                             std::shared_ptr<IrrationalValue> irr) {
    IrrationalValue result;
    result.kind_ = Kind::Product;
    result.product_coeff_ = coeff;
    result.product_irr_ = irr;
    return result;
}

IrrationalValue IrrationalValue::makeSum(std::shared_ptr<IrrationalValue> left,
                                         std::shared_ptr<IrrationalValue> right) {
    IrrationalValue result;
    result.kind_ = Kind::Sum;
    result.sum_left_ = left;
    result.sum_right_ = right;
    return result;
}

// Value::typeName
std::string Value::typeName() const {
    switch (type_) {
        case Type::Int: return "int";
        case Type::Float: return "float";
        case Type::BigInt: return "bigint";
        case Type::Rational: return "rational";
        case Type::Irrational: return "irrational";
        case Type::Complex: return "complex";
        case Type::Bool: return "bool";
        case Type::String: return "string";
        case Type::Null: return "null";
        case Type::Array: return "array";
        case Type::Struct: return "struct";
        case Type::Lambda: return "lambda";
        case Type::Function: return "function";
        case Type::Module: return "module";
        case Type::NativeFunction: return "native_function";
        case Type::CurriedFunction: return "curried_function";
        case Type::MemoizedFunction: return "memoized_function";
    }
    return "unknown";
}

// Value::isTruthy
bool Value::isTruthy() const {
    switch (type_) {
        case Type::Bool:
            return getBool();
        case Type::Null:
            return false;
        case Type::Int:
            return getInt() != 0;
        case Type::Float:
            return getFloat() != 0.0;
        case Type::Complex: {
            auto [re, im] = getComplex();
            return re->isTruthy() || im->isTruthy();
        }
        default:
            return true;
    }
}

// Value::toFloat
double Value::toFloat() const {
    switch (type_) {
        case Type::Int:
            return static_cast<double>(getInt());
        case Type::Float:
            return getFloat();
        case Type::BigInt:
            return getBigInt().get_d();
        case Type::Rational: {
            const auto& r = getRational();
            return r.get_d();
        }
        case Type::Irrational:
            return irrationalToFloat(getIrrational());
        case Type::Complex: {
            auto [re, im] = getComplex();
            double im_float = im->toFloat();
            if (std::abs(im_float) < 1e-10) {
                return re->toFloat();
            }
            throw std::runtime_error("Cannot convert non-real complex number to float");
        }
        default:
            throw std::runtime_error("Cannot convert " + typeName() + " to float");
    }
}

// Value::toInt
int64_t Value::toInt() const {
    switch (type_) {
        case Type::Int:
            return getInt();
        case Type::Float:
            return static_cast<int64_t>(getFloat());
        case Type::Bool:
            return getBool() ? 1 : 0;
        default:
            throw std::runtime_error("Cannot convert " + typeName() + " to int");
    }
}

// Value::operator==
bool Value::operator==(const Value& other) const {
    if (type_ != other.type_) return false;

    switch (type_) {
        case Type::Int:
            return getInt() == other.getInt();
        case Type::Float:
            return std::abs(getFloat() - other.getFloat()) < 1e-10;
        case Type::Bool:
            return getBool() == other.getBool();
        case Type::String:
            return getString() == other.getString();
        case Type::Null:
            return true;
        case Type::Complex: {
            auto [re1, im1] = getComplex();
            auto [re2, im2] = other.getComplex();
            return *re1 == *re2 && *im1 == *im2;
        }
        case Type::BigInt:
            return getBigInt() == other.getBigInt();
        case Type::Rational:
            return getRational() == other.getRational();
        case Type::Irrational:
            return getIrrational() == other.getIrrational();
        default:
            return toString() == other.toString();
    }
}

// Value::toString
std::string Value::toString() const {
    std::ostringstream oss;

    switch (type_) {
        case Type::Int:
            oss << getInt();
            break;
        case Type::Float:
            oss << std::setprecision(15) << getFloat();
            break;
        case Type::BigInt:
            oss << getBigInt().get_str();
            break;
        case Type::Rational: {
            const auto& r = getRational();
            oss << r.get_num().get_str() << "/" << r.get_den().get_str();
            break;
        }
        case Type::Irrational:
            oss << formatIrrational(getIrrational());
            break;
        case Type::Complex: {
            auto [re, im] = getComplex();
            std::string re_str = re->toString();
            std::string im_str = im->toString();

            bool re_is_zero = false;
            if (re->getType() == Type::Int && re->getInt() == 0) re_is_zero = true;
            if (re->getType() == Type::Rational) {
                const auto& r = re->getRational();
                re_is_zero = (r.get_num() == 0);
            }

            bool im_is_zero = false;
            if (im->getType() == Type::Int && im->getInt() == 0) im_is_zero = true;
            if (im->getType() == Type::Rational) {
                const auto& r = im->getRational();
                im_is_zero = (r.get_num() == 0);
            }

            bool im_is_one = false;
            bool im_is_neg_one = false;

            if (im->getType() == Type::Int) {
                im_is_one = (im->getInt() == 1);
                im_is_neg_one = (im->getInt() == -1);
            }
            if (im->getType() == Type::Rational) {
                const auto& r = im->getRational();
                im_is_one = (r.get_num() == 1 && r.get_den() == 1);
                im_is_neg_one = (r.get_num() == -1 && r.get_den() == 1);
            }

            if (im_is_zero) {
                oss << re_str;
            } else if (re_is_zero) {
                if (im_is_one) oss << "i";
                else if (im_is_neg_one) oss << "-i";
                else oss << im_str << "i";
            } else {
                if (im_is_one) oss << re_str << "+i";
                else if (im_is_neg_one) oss << re_str << "-i";
                else {
                    bool im_is_negative = false;
                    if (im->getType() == Type::Int) im_is_negative = (im->getInt() < 0);
                    if (im->getType() == Type::Rational) {
                        const auto& r = im->getRational();
                        im_is_negative = (r.get_num() < 0);
                    }

                    if (im_is_negative) {
                        oss << re_str << im_str << "i";
                    } else {
                        oss << re_str << "+" << im_str << "i";
                    }
                }
            }
            break;
        }
        case Type::Bool:
            oss << (getBool() ? "true" : "false");
            break;
        case Type::String:
            oss << getString();
            break;
        case Type::Null:
            oss << "null";
            break;
        case Type::Array: {
            auto arr = getArray();
            oss << "[";
            for (size_t i = 0; i < arr->size(); ++i) {
                if (i > 0) oss << ", ";
                oss << (*arr)[i].toString();
            }
            oss << "]";
            break;
        }
        case Type::Struct: {
            auto s = getStruct();
            oss << "{";
            size_t i = 0;
            for (const auto& [k, v] : *s) {
                if (i++ > 0) oss << ", ";
                oss << k << " = " << v.toString();
            }
            oss << "}";
            break;
        }
        case Type::Lambda: {
            auto lambda = getLambda();
            oss << "<lambda (" << lambda.params.size() << " params)>";
            break;
        }
        case Type::Function: {
            auto func = getFunction();
            oss << "<function " << func.name << "(";
            for (size_t i = 0; i < func.params.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << func.params[i];
            }
            oss << ")>";
            break;
        }
        case Type::Module:
            oss << "<module>";
            break;
        case Type::NativeFunction: {
            auto nf = getNativeFunction();
            oss << "<native function " << nf.name << ">";
            break;
        }
        case Type::CurriedFunction: {
            auto cf = getCurriedFunction();
            oss << "<curried function " << cf.collected_args.size() 
                << "/" << cf.total_params << " args>";
            break;
        }
        case Type::MemoizedFunction: {
            auto mf = getMemoizedFunction();
            oss << "<memoized function (" << mf.cache->size() << " cached)>";
            break;
        }
    }

    return oss.str();
}

// IrrationalValue::toFloat
double IrrationalValue::toFloat() const {
    switch (kind_) {
        case Kind::Pi:
            return M_PI;
        case Kind::E:
            return M_E;
        case Kind::Sqrt: {
            double val = sqrt_value_->toFloat();
            return std::sqrt(val);
        }
        case Kind::Root: {
            double val = root_value_->toFloat();
            return std::pow(val, 1.0 / root_degree_);
        }
        case Kind::Product: {
            double coeff = product_coeff_->toFloat();
            return coeff * product_irr_->toFloat();
        }
        case Kind::Sum:
            return sum_left_->toFloat() + sum_right_->toFloat();
    }
    return 0.0;
}

// 辅助函数：简化平方根
static std::pair<int64_t, int64_t> simplify_sqrt(const Value* n) {
    if (n->getType() == Value::Type::Int) {
        int64_t num = n->getInt();
        if (num < 0) return {1, num};
        
        int64_t n_val = num;
        int64_t coef = 1;

        for (int64_t i = 2; i * i <= n_val; ++i) {
            int64_t square = i * i;
            while (n_val % square == 0) {
                n_val /= square;
                coef *= i;
            }
        }
        return {coef, n_val};
    }
    return {1, 0};
}

// 辅助函数：格式化平方根
static std::string format_sqrt(const Value* n) {
    if (n->getType() == Value::Type::Int) {
        auto [coef, remaining] = simplify_sqrt(n);
        if (remaining == 1) {
            return std::to_string(coef);
        } else if (coef == 1) {
            return "√" + std::to_string(remaining);
        } else {
            return std::to_string(coef) + "√" + std::to_string(remaining);
        }
    } else if (n->getType() == Value::Type::Irrational) {
        return "√(" + formatIrrational(*static_cast<const IrrationalValue*>(n->get<IrrationalValue>())) + ")";
    } else {
        return "√(" + n->toString() + ")";
    }
}

// 辅助函数：格式化乘积
static std::string format_product(const Value* coeff, const IrrationalValue* irr) {
    if (irr->getKind() == IrrationalValue::Kind::Sqrt) {
        auto sqrt_val = irr->getSqrtValue();
        if (sqrt_val->getType() == Value::Type::Int && sqrt_val->getInt() == 1) {
            return coeff->toString();
        }
    }

    if (coeff->getType() == Value::Type::Irrational) {
        auto coeff_irr = static_cast<const IrrationalValue*>(coeff->get<IrrationalValue>());
        if (coeff_irr->getKind() == IrrationalValue::Kind::Pi &&
            irr->getKind() == IrrationalValue::Kind::Pi) {
            return "π^2";
        }
        if (coeff_irr->getKind() == IrrationalValue::Kind::E &&
            irr->getKind() == IrrationalValue::Kind::E) {
            return "e^2";
        }
    }

    if (irr->getKind() == IrrationalValue::Kind::Product) {
        auto inner_coeff = irr->getProductCoeff();
        auto inner_irr = irr->getProductIrr();

        if (coeff->getType() == Value::Type::Int && 
            inner_coeff->getType() == Value::Type::Int) {
            int64_t a = coeff->getInt();
            int64_t b = inner_coeff->getInt();
            Value combined = Value(static_cast<int64_t>(a * b));
            return format_product(&combined, inner_irr.get());
        }
    }

    std::string coeff_str;
    if (coeff->getType() == Value::Type::Int && coeff->getInt() == 1) {
        return formatIrrational(*irr);
    } else if (coeff->getType() == Value::Type::Int) {
        coeff_str = std::to_string(coeff->getInt());
    } else {
        coeff_str = coeff->toString();
    }

    std::string irr_str = formatIrrational(*irr);

    auto irr_kind = irr->getKind();
    if (irr_kind == IrrationalValue::Kind::Pi ||
        irr_kind == IrrationalValue::Kind::E ||
        irr_kind == IrrationalValue::Kind::Sqrt) {
        return coeff_str + irr_str;
    } else {
        return coeff_str + "*" + irr_str;
    }
}

// 辅助函数：扁平化求和
static void format_sum_flat(const IrrationalValue* irr, std::vector<std::string>& terms) {
    if (irr->getKind() == IrrationalValue::Kind::Sum) {
        format_sum_flat(irr->getSumLeft().get(), terms);
        format_sum_flat(irr->getSumRight().get(), terms);
    } else {
        terms.push_back(formatIrrational(*irr));
    }
}

// IrrationalValue::toString
std::string IrrationalValue::toString() const {
    return formatIrrational(*this);
}

// IrrationalValue::operator==
bool IrrationalValue::operator==(const IrrationalValue& other) const {
    if (kind_ != other.kind_) return false;

    switch (kind_) {
        case Kind::Pi:
        case Kind::E:
            return true;
        case Kind::Sqrt:
            return *sqrt_value_ == *other.sqrt_value_;
        case Kind::Root:
            return root_degree_ == other.root_degree_ &&
                   *root_value_ == *other.root_value_;
        case Kind::Product:
            return *product_coeff_ == *other.product_coeff_ &&
                   *product_irr_ == *other.product_irr_;
        case Kind::Sum:
            return *sum_left_ == *other.sum_left_ &&
                   *sum_right_ == *other.sum_right_;
    }
    return false;
}

// 全局函数：irrationalToFloat
double irrationalToFloat(const IrrationalValue& irr) {
    return irr.toFloat();
}

// 全局函数：formatIrrational
std::string formatIrrational(const IrrationalValue& irr) {
    switch (irr.getKind()) {
        case IrrationalValue::Kind::Sqrt:
            return format_sqrt(irr.getSqrtValue().get());
        case IrrationalValue::Kind::Root: {
            uint32_t degree = irr.getRootDegree();
            if (degree == 2) {
                return format_sqrt(irr.getRootValue().get());
            }
            return std::to_string(degree) + "√" + irr.getRootValue()->toString();
        }
        case IrrationalValue::Kind::Pi:
            return "π";
        case IrrationalValue::Kind::E:
            return "e";
        case IrrationalValue::Kind::Product:
            return format_product(irr.getProductCoeff().get(), 
                                 irr.getProductIrr().get());
        case IrrationalValue::Kind::Sum: {
            std::vector<std::string> terms;
            format_sum_flat(&irr, terms);
            std::string result;
            for (size_t i = 0; i < terms.size(); ++i) {
                if (i > 0) result += "+";
                result += terms[i];
            }
            return result;
        }
    }
    return "";
}

} // namespace rumina
