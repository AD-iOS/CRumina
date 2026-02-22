#pragma once

#include "fwd.h"
#include <string>
#include <vector>
#include <optional>

namespace rumina {

// 错误类型 - 使用独立枚举，不与 ast.h 冲突
enum class RuminaErrorType {
    RuntimeError,
    TypeError,
    IndexError,
    KeyError,
    DivisionByZeroError,
    UndefinedVariableError
};

// 栈帧
struct StackFrame {
    std::string function_name;
    std::string file_name;
    std::optional<size_t> line_number;
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
