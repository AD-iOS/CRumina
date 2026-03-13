#pragma once

#include "fwd.h"
#include <string>
#include <vector>
#include <optional>
#include <sstream>

namespace rumina {

// 错误类型
enum class RuminaErrorType {
    RuntimeError,
    TypeError,
    IndexError,
    KeyError,
    DivisionByZeroError,
    UndefinedVariableError,
    SyntaxError,
    ArityError
};

// 错误类型转字符串
inline std::string error_type_to_string(RuminaErrorType type) {
    switch (type) {
        case RuminaErrorType::RuntimeError: return "RuntimeError";
        case RuminaErrorType::TypeError: return "TypeError";
        case RuminaErrorType::IndexError: return "IndexError";
        case RuminaErrorType::KeyError: return "KeyError";
        case RuminaErrorType::DivisionByZeroError: return "DivisionByZeroError";
        case RuminaErrorType::UndefinedVariableError: return "UndefinedVariableError";
        case RuminaErrorType::SyntaxError: return "SyntaxError";
        case RuminaErrorType::ArityError: return "ArityError";
        default: return "UnknownError";
    }
}

// 栈帧
struct StackFrame {
    std::string function_name;
    std::string file_name;
    std::optional<size_t> line_number;
    
    std::string toString() const {
        std::ostringstream oss;
        oss << "  File \"" << file_name << "\", line ";
        if (line_number.has_value()) {
            oss << line_number.value();
        } else {
            oss << "?";
        }
        oss << ", in " << function_name;
        return oss.str();
    }
};

// 运行时错误
class RuminaError : public std::exception {
public:
    RuminaError(RuminaErrorType type, const std::string& msg);
    
    static RuminaError runtime(const std::string& msg);
    static RuminaError typeError(const std::string& msg);
    static RuminaError indexError(const std::string& msg);
    static RuminaError keyError(const std::string& msg);
    static RuminaError divisionByZero();
    static RuminaError undefinedVariable(const std::string& name);
    static RuminaError syntaxError(const std::string& msg);
    static RuminaError arityError(const std::string& func_name, size_t expected, size_t got);

    void addFrame(const StackFrame& frame);
    
    RuminaErrorType getType() const { return type_; }
    const std::string& getMessage() const { return message_; }
    const std::vector<StackFrame>& getStackTrace() const { return stack_trace_; }
    
    std::string formatError() const;
    const char* what() const noexcept override;

private:
    RuminaErrorType type_;
    std::string message_;
    std::vector<StackFrame> stack_trace_;
    mutable std::string what_cache_;
};

} // namespace rumina