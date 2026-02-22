#pragma once

#include "fwd.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <variant>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <gmpxx.h>

namespace rumina {

// 使用GMP的大整数和有理数
using BigInt = mpz_class;
using BigRational = mpq_class;

// 函数指针类型 - 已经在 fwd.h 中声明
// using NativeFunction = std::function<Value(const std::vector<Value>&)>;

// 无理数表示
class IrrationalValue {
public:
    enum class Kind {
        Sqrt,
        Root,
        Pi,
        E,
        Product,
        Sum
    };

private:
    Kind kind_ = Kind::Pi;
    std::shared_ptr<Value> sqrt_value_;
    uint32_t root_degree_ = 0;
    std::shared_ptr<Value> root_value_;
    std::shared_ptr<Value> product_coeff_;
    std::shared_ptr<IrrationalValue> product_irr_;
    std::shared_ptr<IrrationalValue> sum_left_;
    std::shared_ptr<IrrationalValue> sum_right_;

public:
    IrrationalValue() = default;
    IrrationalValue(const IrrationalValue&) = default;
    IrrationalValue& operator=(const IrrationalValue&) = default;
    
    static IrrationalValue makeSqrt(std::shared_ptr<Value> value);
    static IrrationalValue makeRoot(uint32_t degree, std::shared_ptr<Value> value);
    static IrrationalValue makePi();
    static IrrationalValue makeE();
    static IrrationalValue makeProduct(std::shared_ptr<Value> coeff, 
                                       std::shared_ptr<IrrationalValue> irr);
    static IrrationalValue makeSum(std::shared_ptr<IrrationalValue> left,
                                   std::shared_ptr<IrrationalValue> right);

    Kind getKind() const { return kind_; }
    const std::shared_ptr<Value>& getSqrtValue() const { return sqrt_value_; }
    uint32_t getRootDegree() const { return root_degree_; }
    const std::shared_ptr<Value>& getRootValue() const { return root_value_; }
    const std::shared_ptr<Value>& getProductCoeff() const { return product_coeff_; }
    const std::shared_ptr<IrrationalValue>& getProductIrr() const { return product_irr_; }
    const std::shared_ptr<IrrationalValue>& getSumLeft() const { return sum_left_; }
    const std::shared_ptr<IrrationalValue>& getSumRight() const { return sum_right_; }

    double toFloat() const;
    std::string toString() const;
    bool operator==(const IrrationalValue& other) const;
};

// Lamina运行时值类型
class Value {
public:
    enum class Type {
        Int, Float, BigInt, Rational, Irrational, Complex,
        Bool, String, Null, Array, Struct,
        Lambda, Function, Module, NativeFunction,
        CurriedFunction, MemoizedFunction
    };

    // 数据结构
    struct LambdaData {
        std::vector<std::string> params;
        std::shared_ptr<Stmt> body;
        std::shared_ptr<std::unordered_map<std::string, Value>> closure;
        LambdaData() = default;
        LambdaData(const LambdaData&) = default;
        LambdaData& operator=(const LambdaData&) = default;
    };

    struct FunctionData {
        std::string name;
        std::vector<std::string> params;
        std::shared_ptr<Stmt> body;
        std::vector<std::string> decorators;
        FunctionData() = default;
        FunctionData(const FunctionData&) = default;
        FunctionData& operator=(const FunctionData&) = default;
    };

    struct NativeFunctionData {
        std::string name;
        NativeFunction func;
        NativeFunctionData() = default;
        NativeFunctionData(const NativeFunctionData&) = default;
        NativeFunctionData& operator=(const NativeFunctionData&) = default;
    };

    struct CurriedFunctionData {
        std::shared_ptr<Value> original;
        std::vector<Value> collected_args;
        size_t total_params;
        CurriedFunctionData() = default;
        CurriedFunctionData(const CurriedFunctionData&) = default;
        CurriedFunctionData& operator=(const CurriedFunctionData&) = default;
    };

    struct MemoizedFunctionData {
        std::shared_ptr<Value> original;
        std::shared_ptr<std::unordered_map<std::string, Value>> cache;
        MemoizedFunctionData() = default;
        MemoizedFunctionData(const MemoizedFunctionData&) = default;
        MemoizedFunctionData& operator=(const MemoizedFunctionData&) = default;
    };

    template<typename T>
    const T* get() const {
        return std::get_if<T>(&data_);
    }

    template<typename T>
    T* get() {
        return std::get_if<T>(&data_);
    }

private:
    Type type_;
    
    using Data = std::variant<
        std::monostate,
        int64_t,
        double,
        bool,
        std::string,
        BigInt,
        BigRational,
        IrrationalValue,
        std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>>,
        std::shared_ptr<std::vector<Value>>,
        std::shared_ptr<std::unordered_map<std::string, Value>>,
        LambdaData,
        FunctionData,
        NativeFunctionData,
        CurriedFunctionData,
        MemoizedFunctionData
    >;
    
    Data data_;

public:
    // 构造函数
    Value() : type_(Type::Null), data_(std::monostate{}) {}
    
    // 基本类型构造函数
    Value(int64_t i) : type_(Type::Int), data_(i) {}
    Value(double f) : type_(Type::Float), data_(f) {}
    Value(bool b) : type_(Type::Bool), data_(b) {}
    Value(const std::string& s) : type_(Type::String), data_(s) {}
    Value(const char* s) : type_(Type::String), data_(std::string(s)) {}
    Value(const BigInt& bi) : type_(Type::BigInt), data_(bi) {}
    Value(const BigRational& br) : type_(Type::Rational), data_(br) {}
    Value(const IrrationalValue& irr) : type_(Type::Irrational), data_(irr) {}
    
    // 复数构造函数
    Value(std::shared_ptr<Value> re, std::shared_ptr<Value> im) 
        : type_(Type::Complex), data_(std::make_pair(re, im)) {}

    // 静态工厂方法（用于复杂类型）
    static Value makeArray(std::shared_ptr<std::vector<Value>> arr) {
        Value v;
        v.type_ = Type::Array;
        v.data_ = arr;
        return v;
    }
    
    static Value makeStruct(std::shared_ptr<std::unordered_map<std::string, Value>> s) {
        Value v;
        v.type_ = Type::Struct;
        v.data_ = s;
        return v;
    }
    
    static Value makeModule(std::shared_ptr<std::unordered_map<std::string, Value>> module) {
        Value v;
        v.type_ = Type::Module;
        v.data_ = module;
        return v;
    }
    
    static Value makeLambda(const LambdaData& data) {
        Value v;
        v.type_ = Type::Lambda;
        v.data_ = data;
        return v;
    }
    
    static Value makeFunction(const FunctionData& data) {
        Value v;
        v.type_ = Type::Function;
        v.data_ = data;
        return v;
    }
    
    static Value makeNativeFunction(const std::string& name, NativeFunction func) {
        Value v;
        v.type_ = Type::NativeFunction;
        v.data_ = NativeFunctionData{name, func};
        return v;
    }
    
    static Value makeCurriedFunction(std::shared_ptr<Value> original,
                                     std::vector<Value> args,
                                     size_t total) {
        Value v;
        v.type_ = Type::CurriedFunction;
        v.data_ = CurriedFunctionData{original, std::move(args), total};
        return v;
    }
    
    static Value makeMemoizedFunction(std::shared_ptr<Value> original) {
        Value v;
        v.type_ = Type::MemoizedFunction;
        v.data_ = MemoizedFunctionData{
            original,
            std::make_shared<std::unordered_map<std::string, Value>>()
        };
        return v;
    }

    // 拷贝和移动
    Value(const Value&) = default;
    Value(Value&&) = default;
    Value& operator=(const Value&) = default;
    Value& operator=(Value&&) = default;

    // 类型检查
    Type getType() const { return type_; }
    std::string typeName() const;

    // 值访问
    int64_t getInt() const { 
        if (type_ != Type::Int) throw std::runtime_error("Not an int");
        return std::get<int64_t>(data_); 
    }
    
    double getFloat() const { 
        if (type_ != Type::Float) throw std::runtime_error("Not a float");
        return std::get<double>(data_); 
    }
    
    bool getBool() const { 
        if (type_ != Type::Bool) throw std::runtime_error("Not a bool");
        return std::get<bool>(data_); 
    }
    
    const std::string& getString() const { 
        if (type_ != Type::String) throw std::runtime_error("Not a string");
        return std::get<std::string>(data_); 
    }
    
    const BigInt& getBigInt() const { 
        if (type_ != Type::BigInt) throw std::runtime_error("Not a bigint");
        return std::get<BigInt>(data_); 
    }
    
    const BigRational& getRational() const { 
        if (type_ != Type::Rational) throw std::runtime_error("Not a rational");
        return std::get<BigRational>(data_); 
    }
    
    const IrrationalValue& getIrrational() const { 
        if (type_ != Type::Irrational) throw std::runtime_error("Not an irrational");
        return std::get<IrrationalValue>(data_); 
    }

    std::shared_ptr<std::vector<Value>> getArray() const {
        if (type_ != Type::Array) throw std::runtime_error("Not an array");
        return std::get<std::shared_ptr<std::vector<Value>>>(data_);
    }

    std::shared_ptr<std::unordered_map<std::string, Value>> getStruct() const {
        if (type_ != Type::Struct && type_ != Type::Module) 
            throw std::runtime_error("Not a struct or module");
        return std::get<std::shared_ptr<std::unordered_map<std::string, Value>>>(data_);
    }

    std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>> getComplex() const {
        if (type_ != Type::Complex) throw std::runtime_error("Not a complex");
        return std::get<std::pair<std::shared_ptr<Value>, std::shared_ptr<Value>>>(data_);
    }

    LambdaData getLambda() const {
        if (type_ != Type::Lambda) throw std::runtime_error("Not a lambda");
        return std::get<LambdaData>(data_);
    }

    FunctionData getFunction() const {
        if (type_ != Type::Function) throw std::runtime_error("Not a function");
        return std::get<FunctionData>(data_);
    }

    std::shared_ptr<std::unordered_map<std::string, Value>> getModule() const {
        if (type_ != Type::Module) throw std::runtime_error("Not a module");
        return std::get<std::shared_ptr<std::unordered_map<std::string, Value>>>(data_);
    }

    NativeFunctionData getNativeFunction() const {
        if (type_ != Type::NativeFunction) throw std::runtime_error("Not a native function");
        return std::get<NativeFunctionData>(data_);
    }

    CurriedFunctionData getCurriedFunction() const {
        if (type_ != Type::CurriedFunction) throw std::runtime_error("Not a curried function");
        return std::get<CurriedFunctionData>(data_);
    }

    MemoizedFunctionData getMemoizedFunction() const {
        if (type_ != Type::MemoizedFunction) throw std::runtime_error("Not a memoized function");
        return std::get<MemoizedFunctionData>(data_);
    }

    // 操作
    bool isTruthy() const;
    double toFloat() const;
    int64_t toInt() const;

    // 比较
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const { return !(*this == other); }

    // 字符串表示
    std::string toString() const;

    // 获取数据（用于value.cc中的辅助函数）
    const Data& getData() const { return data_; }
    Data& getData() { return data_; }
};

// 全局辅助函数
double irrationalToFloat(const IrrationalValue& irr);
std::string formatIrrational(const IrrationalValue& irr);

} // namespace rumina

// 哈希支持
namespace std {
    template<>
    struct hash<rumina::Value> {
        size_t operator()(const rumina::Value& v) const {
            return hash<string>()(v.toString());
        }
    };
}
