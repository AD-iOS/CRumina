#include <error.h>
#include <sstream>

namespace rumina {

RuminaError::RuminaError(RuminaErrorType type, const std::string& msg)
    : type_(type), message_(msg) {}

RuminaError RuminaError::runtime(const std::string& msg) {
    return RuminaError(RuminaErrorType::RuntimeError, msg);
}

RuminaError RuminaError::typeError(const std::string& msg) {
    return RuminaError(RuminaErrorType::TypeError, msg);
}

RuminaError RuminaError::indexError(const std::string& msg) {
    return RuminaError(RuminaErrorType::IndexError, msg);
}

RuminaError RuminaError::keyError(const std::string& msg) {
    return RuminaError(RuminaErrorType::KeyError, msg);
}

RuminaError RuminaError::divisionByZero() {
    return RuminaError(RuminaErrorType::DivisionByZeroError, "Division by zero");
}

RuminaError RuminaError::undefinedVariable(const std::string& name) {
    return RuminaError(RuminaErrorType::UndefinedVariableError, 
                       "Undefined variable '" + name + "'");
}

void RuminaError::addFrame(const StackFrame& frame) {
    stack_trace_.push_back(frame);
}

std::string RuminaError::formatError() const {
    std::ostringstream oss;
    
    if (!stack_trace_.empty()) {
        oss << "Traceback (most recent call last):\n";
        
        for (auto it = stack_trace_.rbegin(); it != stack_trace_.rend(); ++it) {
            oss << "  File \"" << it->file_name << "\", line ";
            if (it->line_number.has_value()) {
                oss << it->line_number.value();
            } else {
                oss << "?";
            }
            oss << ", in " << it->function_name << "\n";
        }
    }
    
    switch (type_) {
        case RuminaErrorType::RuntimeError:
            oss << "RuntimeError: ";
            break;
        case RuminaErrorType::TypeError:
            oss << "TypeError: ";
            break;
        case RuminaErrorType::IndexError:
            oss << "IndexError: ";
            break;
        case RuminaErrorType::KeyError:
            oss << "KeyError: ";
            break;
        case RuminaErrorType::DivisionByZeroError:
            oss << "DivisionByZeroError: ";
            break;
        case RuminaErrorType::UndefinedVariableError:
            oss << "UndefinedVariableError: ";
            break;
    }
    
    oss << message_ << "\n";
    
    return oss.str();
}

const char* RuminaError::what() const noexcept {
    what_cache_ = formatError();
    return what_cache_.c_str();
}

} // namespace rumina
