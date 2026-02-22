#pragma once

#include <string>
#include <optional>

namespace rumina {

// Result类型，类似于Rust的Result<T, E>
template<typename T>
class Result {
private:
    bool is_ok_;
    T value_;
    std::string error_;

public:
    Result(T value) : is_ok_(true), value_(std::move(value)) {}
    Result(const std::string& error) : is_ok_(false), error_(error) {}
    Result(const char* error) : is_ok_(false), error_(error) {}
    
    bool is_ok() const { return is_ok_; }
    bool is_error() const { return !is_ok_; }
    
    T value() const { return value_; }
    T& value() { return value_; }
    
    const std::string& error() const { return error_; }
    
    std::optional<T> ok() const {
        if (is_ok_) return value_;
        return std::nullopt;
    }
};

// 辅助函数
template<typename T>
Result<T> Ok(T value) {
    return Result<T>(std::move(value));
}

template<typename T>
Result<T> Err(const std::string& error) {
    return Result<T>(error);
}

template<typename T>
Result<T> Err(const char* error) {
    return Result<T>(std::string(error));
}

// 对于返回void的情况
class VoidResult {
private:
    bool is_ok_;
    std::string error_;

public:
    VoidResult() : is_ok_(true) {}
    VoidResult(const std::string& error) : is_ok_(false), error_(error) {}
    VoidResult(const char* error) : is_ok_(false), error_(error) {}
    
    bool is_ok() const { return is_ok_; }
    bool is_error() const { return !is_ok_; }
    const std::string& error() const { return error_; }
};

inline VoidResult Ok() {
    return VoidResult();
}

} // namespace rumina
