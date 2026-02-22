#include <vm.h>
#include <vm_ops.h>
#include <value_ops.h>
#include <interpreter.h>

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace rumina {

// ByteCode implementation

void ByteCode::emit(OpCode op, std::optional<size_t> line) {
    instructions_.push_back(std::move(op));
    line_numbers_.push_back(line);
}

size_t ByteCode::currentAddress() const {
    return instructions_.size();
}

void ByteCode::patchJump(size_t address, size_t target) {
    if (address >= instructions_.size()) return;
    
    OpCode& op = instructions_[address];
    if (op.type == OpCodeType::Jump ||
        op.type == OpCodeType::JumpIfFalse ||
        op.type == OpCodeType::JumpIfTrue) {
        
        if (op.payload_type == OpCode::PAYLOAD_SIZE) {
            op.payload.size = target;
        }
    }
}

bool ByteCode::valuesEqual(const Value& a, const Value& b) {
    if (a.getType() != b.getType()) return false;
    
    switch (a.getType()) {
        case Value::Type::Int:
            return a.getInt() == b.getInt();
        case Value::Type::Float: {
            double a_val = a.getFloat();
            double b_val = b.getFloat();
            uint64_t a_bits, b_bits;
            std::memcpy(&a_bits, &a_val, sizeof(double));
            std::memcpy(&b_bits, &b_val, sizeof(double));
            return a_bits == b_bits;
        }
        case Value::Type::Bool:
            return a.getBool() == b.getBool();
        case Value::Type::String:
            return a.getString() == b.getString();
        case Value::Type::Null:
            return true;
        default:
            return false;
    }
}

size_t ByteCode::addConstant(const Value& value) {
    for (size_t i = 0; i < constants_.size(); ++i) {
        if (valuesEqual(constants_[i], value)) {
            return i;
        }
    }
    
    size_t index = constants_.size();
    constants_.push_back(value);
    return index;
}

std::string ByteCode::serialize() const {
    std::ostringstream oss;
    
    oss << "RUMINA-BYTECODE-V1\n";
    oss << "CONSTANTS: " << constants_.size() << "\n";
    
    for (size_t i = 0; i < constants_.size(); ++i) {
        oss << "CONST[" << i << "]: ";
        
        const Value& val = constants_[i];
        switch (val.getType()) {
            case Value::Type::Int:
                oss << "Int(" << val.getInt() << ")";
                break;
            case Value::Type::Float:
                oss << "Float(" << std::setprecision(15) << val.getFloat() << ")";
                break;
            case Value::Type::Bool:
                oss << "Bool(" << (val.getBool() ? "true" : "false") << ")";
                break;
            case Value::Type::String: {
                std::string s = val.getString();
                std::string escaped;
                for (char c : s) {
                    switch (c) {
                        case '\\': escaped += "\\\\"; break;
                        case '"': escaped += "\\\""; break;
                        case '\n': escaped += "\\n"; break;
                        case '\r': escaped += "\\r"; break;
                        case '\t': escaped += "\\t"; break;
                        default: escaped += c; break;
                    }
                }
                oss << "String(\"" << escaped << "\")";
                break;
            }
            case Value::Type::Null:
                oss << "Null";
                break;
            default:
                oss << val.toString();
                break;
        }
        oss << "\n";
    }
    
    oss << "\nINSTRUCTIONS:\n";
    
    for (size_t i = 0; i < instructions_.size(); ++i) {
        std::string line_str = line_numbers_[i].has_value() 
            ? std::to_string(line_numbers_[i].value()) 
            : "?";
        
        oss << std::setw(4) << std::setfill('0') << i 
            << " [L" << line_str << "] ";
        
        const OpCode& op = instructions_[i];
        
        switch (op.type) {
            case OpCodeType::PushConst:
                if (op.payload_type == OpCode::PAYLOAD_VALUE)
                    oss << "PushConst(" << op.payload.value.toString() << ")";
                break;
            case OpCodeType::PushConstPooled:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "PushConstPooled(" << op.payload.size << ")";
                break;
            case OpCodeType::PushVar:
                if (op.payload_type == OpCode::PAYLOAD_STRING)
                    oss << "PushVar(" << op.payload.str << ")";
                break;
            case OpCodeType::PopVar:
                if (op.payload_type == OpCode::PAYLOAD_STRING)
                    oss << "PopVar(" << op.payload.str << ")";
                break;
            case OpCodeType::MarkImmutable:
                if (op.payload_type == OpCode::PAYLOAD_STRING)
                    oss << "MarkImmutable(" << op.payload.str << ")";
                break;
            case OpCodeType::Dup: oss << "Dup"; break;
            case OpCodeType::Pop: oss << "Pop"; break;
            case OpCodeType::Add: oss << "Add"; break;
            case OpCodeType::Sub: oss << "Sub"; break;
            case OpCodeType::Mul: oss << "Mul"; break;
            case OpCodeType::Div: oss << "Div"; break;
            case OpCodeType::Mod: oss << "Mod"; break;
            case OpCodeType::Pow: oss << "Pow"; break;
            case OpCodeType::Neg: oss << "Neg"; break;
            case OpCodeType::Factorial: oss << "Factorial"; break;
            case OpCodeType::Not: oss << "Not"; break;
            case OpCodeType::And: oss << "And"; break;
            case OpCodeType::Or: oss << "Or"; break;
            case OpCodeType::Eq: oss << "Eq"; break;
            case OpCodeType::Neq: oss << "Neq"; break;
            case OpCodeType::Gt: oss << "Gt"; break;
            case OpCodeType::Gte: oss << "Gte"; break;
            case OpCodeType::Lt: oss << "Lt"; break;
            case OpCodeType::Lte: oss << "Lte"; break;
            case OpCodeType::Jump:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "Jump(" << op.payload.size << ")";
                break;
            case OpCodeType::JumpIfFalse:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "JumpIfFalse(" << op.payload.size << ")";
                break;
            case OpCodeType::JumpIfTrue:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "JumpIfTrue(" << op.payload.size << ")";
                break;
            case OpCodeType::CallVar:
                if (op.payload_type == OpCode::PAYLOAD_CALL_VAR)
                    oss << "CallVar(" << op.payload.call_var.first << ", " << op.payload.call_var.second << ")";
                break;
            case OpCodeType::Call:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "Call(" << op.payload.size << ")";
                break;
            case OpCodeType::CallMethod:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "CallMethod(" << op.payload.size << ")";
                break;
            case OpCodeType::Return: oss << "Return"; break;
            case OpCodeType::MakeArray:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "MakeArray(" << op.payload.size << ")";
                break;
            case OpCodeType::MakeStruct:
                if (op.payload_type == OpCode::PAYLOAD_SIZE)
                    oss << "MakeStruct(" << op.payload.size << ")";
                break;
            case OpCodeType::Index: oss << "Index"; break;
            case OpCodeType::Member:
                if (op.payload_type == OpCode::PAYLOAD_STRING)
                    oss << "Member(" << op.payload.str << ")";
                break;
            case OpCodeType::IndexAssign: oss << "IndexAssign"; break;
            case OpCodeType::MemberAssign:
                if (op.payload_type == OpCode::PAYLOAD_STRING)
                    oss << "MemberAssign(" << op.payload.str << ")";
                break;
            case OpCodeType::MemberAssignVar:
                if (op.payload_type == OpCode::PAYLOAD_MEMBER_ASSIGN)
                    oss << "MemberAssignVar(" << op.payload.member_assign.first << ", " << op.payload.member_assign.second << ")";
                break;
            case OpCodeType::Break: oss << "Break"; break;
            case OpCodeType::Continue: oss << "Continue"; break;
            case OpCodeType::Halt: oss << "Halt"; break;
            case OpCodeType::DefineFunc:
                if (op.payload_type == OpCode::PAYLOAD_FUNC_INFO) {
                    oss << "DefineFunc(" << op.payload.func_info.name << ", [";
                    for (size_t j = 0; j < op.payload.func_info.params.size(); ++j) {
                        if (j > 0) oss << ",";
                        oss << op.payload.func_info.params[j];
                    }
                    oss << "], " << op.payload.func_info.body_start << ", " << op.payload.func_info.body_end << ", [";
                    for (size_t j = 0; j < op.payload.func_info.decorators.size(); ++j) {
                        if (j > 0) oss << ",";
                        oss << op.payload.func_info.decorators[j];
                    }
                    oss << "])";
                }
                break;
            case OpCodeType::MakeLambda:
                if (op.payload_type == OpCode::PAYLOAD_LAMBDA_INFO) {
                    oss << "MakeLambda([";
                    for (size_t j = 0; j < op.payload.lambda_info.params.size(); ++j) {
                        if (j > 0) oss << ",";
                        oss << op.payload.lambda_info.params[j];
                    }
                    oss << "], " << op.payload.lambda_info.body_start << ", " << op.payload.lambda_info.body_end << ")";
                }
                break;
            case OpCodeType::ConvertType:
                if (op.payload_type == OpCode::PAYLOAD_DECL_TYPE) {
                    const char* dt_str;
                    switch (op.payload.decl_type) {
                        case DeclaredType::Int: dt_str = "Int"; break;
                        case DeclaredType::Float: dt_str = "Float"; break;
                        case DeclaredType::Bool: dt_str = "Bool"; break;
                        case DeclaredType::String: dt_str = "String"; break;
                        case DeclaredType::Rational: dt_str = "Rational"; break;
                        case DeclaredType::Irrational: dt_str = "Irrational"; break;
                        case DeclaredType::Complex: dt_str = "Complex"; break;
                        case DeclaredType::Array: dt_str = "Array"; break;
                        case DeclaredType::BigInt: dt_str = "BigInt"; break;
                        default: dt_str = "Unknown"; break;
                    }
                    oss << "ConvertType(" << dt_str << ")";
                }
                break;
        }
        
        oss << "\n";
    }
    
    return oss.str();
}

ByteCode ByteCode::deserialize(const std::string& input) {
    ByteCode bytecode;
    std::istringstream iss(input);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    
    size_t i = 0;
    
    if (i >= lines.size() || lines[i] != "RUMINA-BYTECODE-V1") {
        throw std::runtime_error("Invalid bytecode header");
    }
    i++;
    
    if (i >= lines.size() || lines[i].rfind("CONSTANTS: ", 0) != 0) {
        throw std::runtime_error("Missing constants section");
    }
    size_t const_count = std::stoul(lines[i].substr(11));
    i++;
    
    for (size_t c = 0; c < const_count; ++c) {
        if (i >= lines.size()) {
            throw std::runtime_error("Unexpected end of constants section");
        }
        
        const std::string& line = lines[i];
        size_t colon_pos = line.find("]: ");
        if (colon_pos == std::string::npos || line.rfind("CONST[", 0) != 0) {
            throw std::runtime_error("Invalid constant format");
        }
        
        std::string value_str = line.substr(colon_pos + 3);
        
        Value value;
        if (value_str.rfind("Int(", 0) == 0 && value_str.back() == ')') {
            int64_t n = std::stoll(value_str.substr(4, value_str.length() - 5));
            value = Value(static_cast<int64_t>(n));
        } else if (value_str.rfind("Float(", 0) == 0 && value_str.back() == ')') {
            double f = std::stod(value_str.substr(6, value_str.length() - 7));
            value = Value(f);
        } else if (value_str.rfind("Bool(", 0) == 0 && value_str.back() == ')') {
            std::string bool_str = value_str.substr(5, value_str.length() - 6);
            bool b = (bool_str == "true");
            value = Value(b);
        } else if (value_str.rfind("String(\"", 0) == 0 && value_str.substr(value_str.length() - 2) == "\")") {
            std::string str = value_str.substr(8, value_str.length() - 10);
            std::string unescaped;
            for (size_t j = 0; j < str.length(); ++j) {
                if (str[j] == '\\' && j + 1 < str.length()) {
                    switch (str[++j]) {
                        case 'n': unescaped += '\n'; break;
                        case 'r': unescaped += '\r'; break;
                        case 't': unescaped += '\t'; break;
                        case '\\': unescaped += '\\'; break;
                        case '"': unescaped += '"'; break;
                        default: unescaped += '\\'; unescaped += str[j]; break;
                    }
                } else {
                    unescaped += str[j];
                }
            }
            value = Value(unescaped);
        } else if (value_str == "Null") {
            value = Value();
        } else {
            throw std::runtime_error("Unsupported value type: " + value_str);
        }
        
        bytecode.constants_.push_back(value);
        i++;
    }
    
    while (i < lines.size() && (lines[i].empty() || lines[i] == "INSTRUCTIONS:")) {
        i++;
    }
    
    while (i < lines.size()) {
        std::string line = lines[i];
        if (line.empty()) {
            i++;
            continue;
        }
        
        size_t space1 = line.find(' ');
        if (space1 == std::string::npos) {
            throw std::runtime_error("Invalid instruction format");
        }
        
        size_t space2 = line.find(' ', space1 + 1);
        if (space2 == std::string::npos) {
            throw std::runtime_error("Invalid instruction format");
        }
        
        std::string line_num_str = line.substr(space1 + 3, space2 - space1 - 4);
        std::optional<size_t> line_num;
        if (line_num_str != "?") {
            line_num = std::stoul(line_num_str);
        }
        
        std::string op_str = line.substr(space2 + 1);
        
        OpCode op(OpCodeType::Halt);
        
        if (op_str == "Dup") op = OpCode(OpCodeType::Dup);
        else if (op_str == "Pop") op = OpCode(OpCodeType::Pop);
        else if (op_str == "Add") op = OpCode(OpCodeType::Add);
        else if (op_str == "Sub") op = OpCode(OpCodeType::Sub);
        else if (op_str == "Mul") op = OpCode(OpCodeType::Mul);
        else if (op_str == "Div") op = OpCode(OpCodeType::Div);
        else if (op_str == "Mod") op = OpCode(OpCodeType::Mod);
        else if (op_str == "Pow") op = OpCode(OpCodeType::Pow);
        else if (op_str == "Neg") op = OpCode(OpCodeType::Neg);
        else if (op_str == "Factorial") op = OpCode(OpCodeType::Factorial);
        else if (op_str == "Not") op = OpCode(OpCodeType::Not);
        else if (op_str == "And") op = OpCode(OpCodeType::And);
        else if (op_str == "Or") op = OpCode(OpCodeType::Or);
        else if (op_str == "Eq") op = OpCode(OpCodeType::Eq);
        else if (op_str == "Neq") op = OpCode(OpCodeType::Neq);
        else if (op_str == "Gt") op = OpCode(OpCodeType::Gt);
        else if (op_str == "Gte") op = OpCode(OpCodeType::Gte);
        else if (op_str == "Lt") op = OpCode(OpCodeType::Lt);
        else if (op_str == "Lte") op = OpCode(OpCodeType::Lte);
        else if (op_str == "Return") op = OpCode(OpCodeType::Return);
        else if (op_str == "Index") op = OpCode(OpCodeType::Index);
        else if (op_str == "IndexAssign") op = OpCode(OpCodeType::IndexAssign);
        else if (op_str == "Break") op = OpCode(OpCodeType::Break);
        else if (op_str == "Continue") op = OpCode(OpCodeType::Continue);
        else if (op_str == "Halt") op = OpCode(OpCodeType::Halt);
        else if (op_str.rfind("PushConst(", 0) == 0) {
            std::string val_str = op_str.substr(10, op_str.length() - 11);
            if (val_str.rfind("Int(", 0) == 0) {
                int64_t n = std::stoll(val_str.substr(4, val_str.length() - 5));
                op = OpCode(OpCodeType::PushConst, Value(static_cast<int64_t>(n)));
            } else if (val_str.rfind("Float(", 0) == 0) {
                double f = std::stod(val_str.substr(6, val_str.length() - 7));
                op = OpCode(OpCodeType::PushConst, Value(f));
            } else {
                op = OpCode(OpCodeType::PushConst, Value());
            }
        } else if (op_str.rfind("PushConstPooled(", 0) == 0) {
            size_t idx = std::stoul(op_str.substr(16, op_str.length() - 17));
            op = OpCode(OpCodeType::PushConstPooled, idx);
        } else if (op_str.rfind("PushVar(", 0) == 0) {
            std::string name = op_str.substr(8, op_str.length() - 9);
            op = OpCode(OpCodeType::PushVar, name);
        } else if (op_str.rfind("PopVar(", 0) == 0) {
            std::string name = op_str.substr(7, op_str.length() - 8);
            op = OpCode(OpCodeType::PopVar, name);
        } else if (op_str.rfind("MarkImmutable(", 0) == 0) {
            std::string name = op_str.substr(14, op_str.length() - 15);
            op = OpCode(OpCodeType::MarkImmutable, name);
        } else if (op_str.rfind("Jump(", 0) == 0) {
            size_t addr = std::stoul(op_str.substr(5, op_str.length() - 6));
            op = OpCode(OpCodeType::Jump, addr);
        } else if (op_str.rfind("JumpIfFalse(", 0) == 0) {
            size_t addr = std::stoul(op_str.substr(12, op_str.length() - 13));
            op = OpCode(OpCodeType::JumpIfFalse, addr);
        } else if (op_str.rfind("JumpIfTrue(", 0) == 0) {
            size_t addr = std::stoul(op_str.substr(11, op_str.length() - 12));
            op = OpCode(OpCodeType::JumpIfTrue, addr);
        } else if (op_str.rfind("CallVar(", 0) == 0) {
            std::string args_str = op_str.substr(8, op_str.length() - 9);
            size_t comma_pos = args_str.find(", ");
            std::string name = args_str.substr(0, comma_pos);
            size_t argc = std::stoul(args_str.substr(comma_pos + 2));
            op = OpCode(OpCodeType::CallVar, std::make_pair(name, argc));
        } else if (op_str.rfind("Call(", 0) == 0) {
            size_t argc = std::stoul(op_str.substr(5, op_str.length() - 6));
            op = OpCode(OpCodeType::Call, argc);
        } else if (op_str.rfind("CallMethod(", 0) == 0) {
            size_t argc = std::stoul(op_str.substr(11, op_str.length() - 12));
            op = OpCode(OpCodeType::CallMethod, argc);
        } else if (op_str.rfind("MakeArray(", 0) == 0) {
            size_t size = std::stoul(op_str.substr(10, op_str.length() - 11));
            op = OpCode(OpCodeType::MakeArray, size);
        } else if (op_str.rfind("MakeStruct(", 0) == 0) {
            size_t size = std::stoul(op_str.substr(11, op_str.length() - 12));
            op = OpCode(OpCodeType::MakeStruct, size);
        } else if (op_str.rfind("Member(", 0) == 0) {
            std::string name = op_str.substr(7, op_str.length() - 8);
            op = OpCode(OpCodeType::Member, name);
        } else if (op_str.rfind("MemberAssign(", 0) == 0) {
            std::string name = op_str.substr(13, op_str.length() - 14);
            op = OpCode(OpCodeType::MemberAssign, name);
        } else if (op_str.rfind("MemberAssignVar(", 0) == 0) {
            std::string args_str = op_str.substr(16, op_str.length() - 17);
            size_t comma_pos = args_str.find(", ");
            std::string var_name = args_str.substr(0, comma_pos);
            std::string member_name = args_str.substr(comma_pos + 2);
            op = OpCode(OpCodeType::MemberAssignVar, std::make_pair(var_name, member_name));
        }
        
        bytecode.instructions_.push_back(op);
        bytecode.line_numbers_.push_back(line_num);
        
        i++;
    }
    
    return bytecode;
}

// VM implementation

VM::VM(std::shared_ptr<std::unordered_map<std::string, Value>> globals)
    : globals_(globals) {
    stack_.reserve(256);
    call_stack_.reserve(64);
    loop_stack_.reserve(8);
}

void VM::load(ByteCode bytecode) {
    bytecode_ = std::move(bytecode);
    ip_ = 0;
    halted_ = false;
}

Result<std::optional<Value>> VM::run() {
    while (!halted_ && ip_ < bytecode_.getInstructions().size()) {
        size_t current_ip = ip_;
        ip_++;
        
        try {
            executeInstructionAt(current_ip);
        } catch (const std::exception& e) {
            return Err<std::optional<Value>>(e.what());
        }
    }
    
    if (!stack_.empty()) {
        Value result = stack_.back();
        stack_.pop_back();
        return Ok(std::optional<Value>(result));
    }
    return Ok(std::optional<Value>(std::nullopt));
}

void VM::executeInstructionAt(size_t ip) {
    const OpCode& op = bytecode_.getInstructions()[ip];
    
    switch (op.type) {
        case OpCodeType::PushConst: {
            if (op.payload_type == OpCode::PAYLOAD_VALUE) {
                stack_.push_back(op.payload.value);
            }
            break;
        }
        
        case OpCodeType::PushConstPooled: {
            if (op.payload_type == OpCode::PAYLOAD_SIZE) {
                size_t index = op.payload.size;
                if (index < bytecode_.getConstants().size()) {
                    stack_.push_back(bytecode_.getConstants()[index]);
                } else {
                    throw std::runtime_error("Invalid constant pool index");
                }
            }
            break;
        }
        
        case OpCodeType::PushVar: {
            if (op.payload_type == OpCode::PAYLOAD_STRING) {
                stack_.push_back(getVariable(op.payload.str));
            }
            break;
        }
        
        case OpCodeType::PopVar: {
            if (op.payload_type == OpCode::PAYLOAD_STRING) {
                if (stack_.empty()) throw std::runtime_error("Stack underflow");
                Value val = stack_.back();
                stack_.pop_back();
                setVariableChecked(op.payload.str, val);
            }
            break;
        }
        
        case OpCodeType::MarkImmutable: {
            if (op.payload_type == OpCode::PAYLOAD_STRING) {
                if (call_stack_.empty()) {
                    immutable_globals_.insert(op.payload.str);
                } else {
                    immutable_locals_.insert(op.payload.str);
                }
            }
            break;
        }
        
        case OpCodeType::Dup: {
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            stack_.push_back(stack_.back());
            break;
        }
        
        case OpCodeType::Pop: {
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            stack_.pop_back();
            break;
        }
        
        case OpCodeType::Add: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Add, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Sub: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Sub, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Mul: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Mul, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Div: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Div, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Mod: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Mod, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Pow: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Pow, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Neg: {
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value val = stack_.back();
            stack_.pop_back();
            auto result = value_unary_op(UnaryOp::Neg, val);
            if (result.is_error()) throw std::runtime_error(result.error());
            stack_.push_back(result.value());
            break;
        }
        
        case OpCodeType::Not: {
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value val = stack_.back();
            stack_.pop_back();
            auto result = value_unary_op(UnaryOp::Not, val);
            if (result.is_error()) throw std::runtime_error(result.error());
            stack_.push_back(result.value());
            break;
        }
        
        case OpCodeType::Factorial: {
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value val = stack_.back();
            stack_.pop_back();
            auto result = value_unary_op(UnaryOp::Factorial, val);
            if (result.is_error()) throw std::runtime_error(result.error());
            stack_.push_back(result.value());
            break;
        }
        
        case OpCodeType::Eq: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Equal, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Neq: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::NotEqual, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Gt: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Greater, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Gte: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::GreaterEq, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Lt: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Less, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Lte: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::LessEq, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::And: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::And, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Or: {
            binaryOp([](const Value& a, const Value& b) {
                auto result = value_binary_op(a, BinOp::Or, b);
                if (result.is_error()) throw std::runtime_error(result.error());
                return result.value();
            });
            break;
        }
        
        case OpCodeType::Jump: {
            if (op.payload_type == OpCode::PAYLOAD_SIZE) {
                ip_ = op.payload.size;
            }
            break;
        }
        
        case OpCodeType::JumpIfFalse: {
            if (op.payload_type == OpCode::PAYLOAD_SIZE) {
                if (stack_.empty()) throw std::runtime_error("Stack underflow");
                Value cond = stack_.back();
                stack_.pop_back();
                if (!cond.isTruthy()) {
                    ip_ = op.payload.size;
                }
            }
            break;
        }
        
        case OpCodeType::JumpIfTrue: {
            if (op.payload_type == OpCode::PAYLOAD_SIZE) {
                if (stack_.empty()) throw std::runtime_error("Stack underflow");
                Value cond = stack_.back();
                stack_.pop_back();
                if (cond.isTruthy()) {
                    ip_ = op.payload.size;
                }
            }
            break;
        }
        
        case OpCodeType::MakeArray: {
            if (op.payload_type == OpCode::PAYLOAD_SIZE) {
                size_t count = op.payload.size;
                std::vector<Value> elements;
                elements.reserve(count);
                
                for (size_t i = 0; i < count; ++i) {
                    if (stack_.empty()) throw std::runtime_error("Stack underflow");
                    elements.push_back(stack_.back());
                    stack_.pop_back();
                }
                
                std::reverse(elements.begin(), elements.end());
                stack_.push_back(Value::makeArray(
                    std::make_shared<std::vector<Value>>(std::move(elements))));
            }
            break;
        }
        
        case OpCodeType::Index: {
            if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
            
            Value index = stack_.back();
            stack_.pop_back();
            Value array = stack_.back();
            stack_.pop_back();
            
            if (array.getType() == Value::Type::Array) {
                auto arr = array.getArray();
                if (index.getType() != Value::Type::Int) {
                    throw std::runtime_error("Array index must be an integer");
                }
                
                int64_t idx = index.getInt();
                size_t len = arr->size();
                
                if (idx < 0) {
                    idx = len + idx;
                }
                
                if (idx < 0 || static_cast<size_t>(idx) >= len) {
                    throw std::runtime_error("Array index out of bounds");
                }
                
                stack_.push_back((*arr)[idx]);
            } else if (array.getType() == Value::Type::String) {
                const std::string& s = array.getString();
                if (index.getType() != Value::Type::Int) {
                    throw std::runtime_error("String index must be an integer");
                }
                
                int64_t idx = index.getInt();
                size_t len = s.length();
                
                if (idx < 0) {
                    idx = len + idx;
                }
                
                if (idx < 0 || static_cast<size_t>(idx) >= len) {
                    throw std::runtime_error("String index out of bounds");
                }
                
                stack_.push_back(Value(std::string(1, s[idx])));
            } else {
                throw std::runtime_error("Cannot index type " + array.typeName());
            }
            break;
        }
        
        case OpCodeType::Member: {
            size_t cache_addr = ip;
            
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value object = stack_.back();
            stack_.pop_back();
            
            if (op.payload_type != OpCode::PAYLOAD_STRING) break;
            const std::string& member = op.payload.str;
            
            if (object.getType() == Value::Type::Struct ||
                object.getType() == Value::Type::Module) {
                
                auto struct_map = (object.getType() == Value::Type::Struct) 
                    ? object.getStruct() : object.getModule();
                
                auto it = struct_map->find(member);
                if (it != struct_map->end()) {
                    auto cache_it = member_cache_.find(cache_addr);
                    if (cache_it != member_cache_.end()) {
                        cache_it->second.hits++;
                    } else {
                        member_cache_[cache_addr] = InlineCache{member, 0, 0};
                    }
                    
                    stack_.push_back(it->second);
                } else {
                    auto cache_it = member_cache_.find(cache_addr);
                    if (cache_it != member_cache_.end()) {
                        cache_it->second.misses++;
                    } else {
                        member_cache_[cache_addr] = InlineCache{member, 0, 1};
                    }
                    
                    throw std::runtime_error(object.typeName() + 
                        " does not have member '" + member + "'");
                }
            } else {
                auto cache_it = member_cache_.find(cache_addr);
                if (cache_it != member_cache_.end()) {
                    cache_it->second.misses++;
                } else {
                    member_cache_[cache_addr] = InlineCache{member, 0, 1};
                }
                
                throw std::runtime_error("Cannot access member of type " + object.typeName());
            }
            break;
        }
        
        case OpCodeType::Return: {
            if (!call_stack_.empty()) {
                CallFrame frame = call_stack_.back();
                call_stack_.pop_back();
                
                recursion_depth_ = recursion_depth_ > 0 ? recursion_depth_ - 1 : 0;
                
                ip_ = frame.return_address;
                locals_ = std::move(frame.locals);
                immutable_locals_ = std::move(frame.immutable_locals);
            } else {
                halted_ = true;
            }
            break;
        }
        
        case OpCodeType::Break: {
            if (!loop_stack_.empty()) {
                ip_ = loop_stack_.back().second;
            } else {
                throw std::runtime_error("Break outside of loop");
            }
            break;
        }
        
        case OpCodeType::Continue: {
            if (!loop_stack_.empty()) {
                ip_ = loop_stack_.back().first;
            } else {
                throw std::runtime_error("Continue outside of loop");
            }
            break;
        }
        
        case OpCodeType::DefineFunc: {
            if (op.payload_type == OpCode::PAYLOAD_FUNC_INFO) {
                const FuncDefInfo& info = op.payload.func_info;
                functions_[info.name] = FuncDefInfo{info.name, info.params, 
                                                    info.body_start, info.body_end, info.decorators};
                
                Value::FunctionData func_data;
                func_data.name = info.name;
                func_data.params = info.params;
                func_data.body = nullptr;
                func_data.decorators = info.decorators;
                
                Value func = Value::makeFunction(func_data);
                (*globals_)[info.name] = func;
            }
            break;
        }
        
        case OpCodeType::CallVar: {
            if (op.payload_type != OpCode::PAYLOAD_CALL_VAR) break;
            
            const std::string& func_name = op.payload.call_var.first;
            size_t arg_count = op.payload.call_var.second;
            
            Value func = getVariable(func_name);
            
            std::vector<Value> args;
            args.reserve(arg_count);
            for (size_t i = 0; i < arg_count; ++i) {
                if (stack_.empty()) throw std::runtime_error("Stack underflow");
                args.push_back(stack_.back());
                stack_.pop_back();
            }
            std::reverse(args.begin(), args.end());
            
            if (func.getType() == Value::Type::NativeFunction) {
                auto nf = func.getNativeFunction();
                Value result = nf.func(args);
                stack_.push_back(result);
            } else if (func.getType() == Value::Type::Function) {
                auto f = func.getFunction();
                
                auto it = functions_.find(f.name);
                if (it == functions_.end()) {
                    throw std::runtime_error("Function '" + f.name + "' not found");
                }
                
                const FuncDefInfo& info = it->second;
                
                if (args.size() != info.params.size()) {
                    throw std::runtime_error("Function '" + func_name + 
                        "' expects " + std::to_string(info.params.size()) + 
                        " arguments, got " + std::to_string(args.size()));
                }
                
                if (recursion_depth_ >= MAX_RECURSION_DEPTH) {
                    throw std::runtime_error("Maximum recursion depth exceeded");
                }
                
                size_t body_start = info.body_start;
                
                CallFrame frame;
                frame.return_address = ip_;
                frame.base_pointer = stack_.size();
                frame.function_name = func_name;
                frame.locals = std::move(locals_);
                frame.immutable_locals = std::move(immutable_locals_);
                
                call_stack_.push_back(std::move(frame));
                recursion_depth_++;
                
                locals_.clear();
                for (size_t i = 0; i < info.params.size(); ++i) {
                    locals_[info.params[i]] = args[i];
                }
                immutable_locals_.clear();
                
                ip_ = body_start;
            } else if (func.getType() == Value::Type::Lambda) {
                auto lambda = func.getLambda();
                
                if (args.size() != lambda.params.size()) {
                    throw std::runtime_error("Lambda expects " + 
                        std::to_string(lambda.params.size()) + 
                        " arguments, got " + std::to_string(args.size()));
                }
                
                if (recursion_depth_ >= MAX_RECURSION_DEPTH) {
                    throw std::runtime_error("Maximum recursion depth exceeded");
                }
                
                std::string lambda_id;
                if (lambda.body) {
                    auto include_stmt = dynamic_cast<IncludeStmt*>(lambda.body.get());
                    if (include_stmt) {
                        lambda_id = include_stmt->path;
                    } else {
                        lambda_id = "__lambda_" + std::to_string(reinterpret_cast<uintptr_t>(lambda.body.get()));
                    }
                } else {
                    for (const auto& [name, _] : functions_) {
                        if (name.rfind("__lambda_", 0) == 0) {
                            lambda_id = name;
                            break;
                        }
                    }
                }
                
                auto it = functions_.find(lambda_id);
                if (it == functions_.end()) {
                    throw std::runtime_error("Lambda not found");
                }
                
                const FuncDefInfo& info = it->second;
                
                CallFrame frame;
                frame.return_address = ip_;
                frame.base_pointer = stack_.size();
                frame.function_name = lambda_id;
                frame.locals = std::move(locals_);
                frame.immutable_locals = std::move(immutable_locals_);
                
                call_stack_.push_back(std::move(frame));
                recursion_depth_++;
                
                locals_.clear();
                if (lambda.closure) {
                    for (const auto& [k, v] : *lambda.closure) {
                        locals_[k] = v;
                    }
                }
                for (size_t i = 0; i < lambda.params.size(); ++i) {
                    locals_[lambda.params[i]] = args[i];
                }
                immutable_locals_.clear();
                
                ip_ = info.body_start;
            } else {
                throw std::runtime_error("Cannot call type " + func.typeName());
            }
            break;
        }
        
        case OpCodeType::Call: {
            if (op.payload_type != OpCode::PAYLOAD_SIZE) break;
            size_t arg_count = op.payload.size;
            
            std::vector<Value> args;
            args.reserve(arg_count);
            for (size_t i = 0; i < arg_count; ++i) {
                if (stack_.empty()) throw std::runtime_error("Stack underflow");
                args.push_back(stack_.back());
                stack_.pop_back();
            }
            std::reverse(args.begin(), args.end());
            
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value func = stack_.back();
            stack_.pop_back();
            
            if (func.getType() == Value::Type::NativeFunction) {
                auto nf = func.getNativeFunction();
                Value result = nf.func(args);
                stack_.push_back(result);
            } else if (func.getType() == Value::Type::Function) {
                auto f = func.getFunction();
                
                auto it = functions_.find(f.name);
                if (it == functions_.end()) {
                    throw std::runtime_error("Function '" + f.name + "' not found");
                }
                
                const FuncDefInfo& info = it->second;
                
                if (args.size() != info.params.size()) {
                    throw std::runtime_error("Function '" + f.name + 
                        "' expects " + std::to_string(info.params.size()) + 
                        " arguments, got " + std::to_string(args.size()));
                }
                
                if (recursion_depth_ >= MAX_RECURSION_DEPTH) {
                    throw std::runtime_error("Maximum recursion depth exceeded");
                }
                
                CallFrame frame;
                frame.return_address = ip_;
                frame.base_pointer = stack_.size();
                frame.function_name = f.name;
                frame.locals = std::move(locals_);
                frame.immutable_locals = std::move(immutable_locals_);
                
                call_stack_.push_back(std::move(frame));
                recursion_depth_++;
                
                locals_.clear();
                for (size_t i = 0; i < info.params.size(); ++i) {
                    locals_[info.params[i]] = args[i];
                }
                immutable_locals_.clear();
                
                ip_ = info.body_start;
            } else if (func.getType() == Value::Type::Lambda) {
                auto lambda = func.getLambda();
                
                if (args.size() != lambda.params.size()) {
                    throw std::runtime_error("Lambda expects " + 
                        std::to_string(lambda.params.size()) + 
                        " arguments, got " + std::to_string(args.size()));
                }
                
                if (recursion_depth_ >= MAX_RECURSION_DEPTH) {
                    throw std::runtime_error("Maximum recursion depth exceeded");
                }
                
                std::string lambda_id;
                if (lambda.body) {
                    auto include_stmt = dynamic_cast<IncludeStmt*>(lambda.body.get());
                    if (include_stmt) {
                        lambda_id = include_stmt->path;
                    } else {
                        lambda_id = "__lambda_" + std::to_string(reinterpret_cast<uintptr_t>(lambda.body.get()));
                    }
                } else {
                    for (const auto& [name, _] : functions_) {
                        if (name.rfind("__lambda_", 0) == 0) {
                            lambda_id = name;
                            break;
                        }
                    }
                }
                
                auto it = functions_.find(lambda_id);
                if (it == functions_.end()) {
                    throw std::runtime_error("Lambda not found");
                }
                
                const FuncDefInfo& info = it->second;
                
                CallFrame frame;
                frame.return_address = ip_;
                frame.base_pointer = stack_.size();
                frame.function_name = lambda_id;
                frame.locals = std::move(locals_);
                frame.immutable_locals = std::move(immutable_locals_);
                
                call_stack_.push_back(std::move(frame));
                recursion_depth_++;
                
                locals_.clear();
                if (lambda.closure) {
                    for (const auto& [k, v] : *lambda.closure) {
                        locals_[k] = v;
                    }
                }
                for (size_t i = 0; i < lambda.params.size(); ++i) {
                    locals_[lambda.params[i]] = args[i];
                }
                immutable_locals_.clear();
                
                ip_ = info.body_start;
            } else {
                throw std::runtime_error("Cannot call type " + func.typeName());
            }
            break;
        }
        
        case OpCodeType::CallMethod: {
            if (op.payload_type != OpCode::PAYLOAD_SIZE) break;
            size_t arg_count = op.payload.size;
            
            std::vector<Value> args;
            args.reserve(arg_count);
            for (size_t i = 0; i < arg_count; ++i) {
                if (stack_.empty()) throw std::runtime_error("Stack underflow");
                args.push_back(stack_.back());
                stack_.pop_back();
            }
            std::reverse(args.begin(), args.end());
            
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value method = stack_.back();
            stack_.pop_back();
            
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value object = stack_.back();
            stack_.pop_back();
            
            if (method.getType() == Value::Type::Lambda) {
                auto lambda = method.getLambda();
                
                if (args.size() != lambda.params.size()) {
                    throw std::runtime_error("Method expects " + 
                        std::to_string(lambda.params.size()) + 
                        " arguments, got " + std::to_string(args.size()));
                }
                
                if (recursion_depth_ >= MAX_RECURSION_DEPTH) {
                    throw std::runtime_error("Maximum recursion depth exceeded");
                }
                
                std::string lambda_id;
                if (lambda.body) {
                    auto include_stmt = dynamic_cast<IncludeStmt*>(lambda.body.get());
                    if (include_stmt) {
                        lambda_id = include_stmt->path;
                    } else {
                        lambda_id = "__lambda_" + std::to_string(reinterpret_cast<uintptr_t>(lambda.body.get()));
                    }
                } else {
                    for (const auto& [name, _] : functions_) {
                        if (name.rfind("__lambda_", 0) == 0) {
                            lambda_id = name;
                            break;
                        }
                    }
                }
                
                auto it = functions_.find(lambda_id);
                if (it == functions_.end()) {
                    throw std::runtime_error("Lambda not found");
                }
                
                const FuncDefInfo& info = it->second;
                
                CallFrame frame;
                frame.return_address = ip_;
                frame.base_pointer = stack_.size();
                frame.function_name = lambda_id;
                frame.locals = std::move(locals_);
                frame.immutable_locals = std::move(immutable_locals_);
                
                call_stack_.push_back(std::move(frame));
                recursion_depth_++;
                
                locals_.clear();
                if (lambda.closure) {
                    for (const auto& [k, v] : *lambda.closure) {
                        locals_[k] = v;
                    }
                }
                locals_["self"] = object;
                for (size_t i = 0; i < lambda.params.size(); ++i) {
                    locals_[lambda.params[i]] = args[i];
                }
                immutable_locals_.clear();
                
                ip_ = info.body_start;
            } else if (method.getType() == Value::Type::Function) {
                auto f = method.getFunction();
                
                auto it = functions_.find(f.name);
                if (it == functions_.end()) {
                    throw std::runtime_error("Function '" + f.name + "' not found");
                }
                
                const FuncDefInfo& info = it->second;
                
                if (args.size() != info.params.size()) {
                    throw std::runtime_error("Function '" + f.name + 
                        "' expects " + std::to_string(info.params.size()) + 
                        " arguments, got " + std::to_string(args.size()));
                }
                
                if (recursion_depth_ >= MAX_RECURSION_DEPTH) {
                    throw std::runtime_error("Maximum recursion depth exceeded");
                }
                
                CallFrame frame;
                frame.return_address = ip_;
                frame.base_pointer = stack_.size();
                frame.function_name = f.name;
                frame.locals = std::move(locals_);
                frame.immutable_locals = std::move(immutable_locals_);
                
                call_stack_.push_back(std::move(frame));
                recursion_depth_++;
                
                locals_.clear();
                locals_.reserve(info.params.size() + 1);
                locals_["self"] = object;
                for (size_t i = 0; i < info.params.size(); ++i) {
                    locals_[info.params[i]] = args[i];
                }
                immutable_locals_.clear();
                
                ip_ = info.body_start;
            } else if (method.getType() == Value::Type::NativeFunction) {
                auto nf = method.getNativeFunction();
                std::vector<Value> native_args;
                native_args.reserve(args.size() + 1);
                native_args.push_back(object);
                native_args.insert(native_args.end(), args.begin(), args.end());
                Value result = nf.func(native_args);
                stack_.push_back(result);
            } else {
                throw std::runtime_error("Cannot call method of type " + method.typeName());
            }
            break;
        }
        
        case OpCodeType::MakeStruct: {
            if (op.payload_type == OpCode::PAYLOAD_SIZE) {
                size_t field_count = op.payload.size;
                auto fields = std::make_shared<std::unordered_map<std::string, Value>>();
                
                for (size_t i = 0; i < field_count; ++i) {
                    if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                    
                    Value value = stack_.back();
                    stack_.pop_back();
                    Value key_val = stack_.back();
                    stack_.pop_back();
                    
                    if (key_val.getType() != Value::Type::String) {
                        throw std::runtime_error("Struct key must be a string");
                    }
                    
                    (*fields)[key_val.getString()] = value;
                }
                
                stack_.push_back(Value::makeStruct(fields));
            }
            break;
        }
        
        case OpCodeType::MakeLambda: {
            if (op.payload_type != OpCode::PAYLOAD_LAMBDA_INFO) break;
            
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value lambda_id_val = stack_.back();
            stack_.pop_back();
            
            if (lambda_id_val.getType() != Value::Type::String) {
                throw std::runtime_error("Expected lambda ID as string");
            }
            std::string lambda_id = lambda_id_val.getString();
            
            auto closure = std::make_shared<std::unordered_map<std::string, Value>>();
            if (!locals_.empty()) {
                *closure = locals_;
            } else {
                closure = globals_;
            }
            
            Value::LambdaData lambda_data;
            lambda_data.params = op.payload.lambda_info.params;
            lambda_data.body = std::make_shared<IncludeStmt>(lambda_id);
            lambda_data.closure = closure;
            
            stack_.push_back(Value::makeLambda(lambda_data));
            break;
        }
        
        case OpCodeType::MemberAssign: {
            if (op.payload_type != OpCode::PAYLOAD_STRING) break;
            
            if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
            
            Value value = stack_.back();
            stack_.pop_back();
            Value object = stack_.back();
            stack_.pop_back();
            
            const std::string& member = op.payload.str;
            
            if (object.getType() == Value::Type::Struct ||
                object.getType() == Value::Type::Module) {
                
                auto struct_map = (object.getType() == Value::Type::Struct) 
                    ? object.getStruct() : object.getModule();
                
                (*struct_map)[member] = value;
            } else {
                throw std::runtime_error("Cannot assign member to " + object.typeName());
            }
            break;
        }
        
        case OpCodeType::MemberAssignVar: {
            if (op.payload_type != OpCode::PAYLOAD_MEMBER_ASSIGN) break;
            
            const std::string& var_name = op.payload.member_assign.first;
            const std::string& member = op.payload.member_assign.second;
            
            ensureMutable(var_name);
            
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value value = stack_.back();
            stack_.pop_back();
            
            Value object = getVariable(var_name);
            
            if (object.getType() == Value::Type::Struct ||
                object.getType() == Value::Type::Module) {
                
                auto struct_map = (object.getType() == Value::Type::Struct) 
                    ? object.getStruct() : object.getModule();
                
                (*struct_map)[member] = value;
            } else if (object.getType() == Value::Type::Null) {
                ensureMutable(var_name);
                auto new_struct = std::make_shared<std::unordered_map<std::string, Value>>();
                (*new_struct)[member] = value;
                setVariableChecked(var_name, Value::makeStruct(new_struct));
            } else {
                throw std::runtime_error("Cannot assign member to " + object.typeName());
            }
            break;
        }
        
        case OpCodeType::IndexAssign:
            throw std::runtime_error("IndexAssign not yet implemented");
        
        case OpCodeType::ConvertType: {
            if (op.payload_type != OpCode::PAYLOAD_DECL_TYPE) break;
            
            if (stack_.empty()) throw std::runtime_error("Stack underflow");
            Value val = stack_.back();
            stack_.pop_back();
            
            Value converted = convertToType(val, op.payload.decl_type);
            stack_.push_back(converted);
            break;
        }
        
        case OpCodeType::Halt:
            halted_ = true;
            break;
    }
}

template<typename F>
void VM::binaryOp(F&& f) {
    if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
    
    Value right = stack_.back();
    stack_.pop_back();
    Value left = stack_.back();
    stack_.pop_back();
    
    Value result = f(left, right);
    stack_.push_back(result);
}

Value VM::getVariable(const std::string& name) const {
    auto it = locals_.find(name);
    if (it != locals_.end()) {
        return it->second;
    }
    
    auto git = globals_->find(name);
    if (git != globals_->end()) {
        return git->second;
    }
    
    throw std::runtime_error("Undefined variable: " + name);
}

void VM::setVariable(const std::string& name, const Value& value) {
    if (call_stack_.empty()) {
        (*globals_)[name] = value;
    } else {
        locals_[name] = value;
    }
}

void VM::ensureMutable(const std::string& name) const {
    if (call_stack_.empty()) {
        if (immutable_globals_.find(name) != immutable_globals_.end()) {
            throw std::runtime_error("Cannot assign to immutable variable '" + name + "'");
        }
    } else {
        if (immutable_locals_.find(name) != immutable_locals_.end()) {
            throw std::runtime_error("Cannot assign to immutable variable '" + name + "'");
        }
    }
}

void VM::setVariableChecked(const std::string& name, const Value& value) {
    ensureMutable(name);
    setVariable(name, value);
}

Value VM::convertToType(const Value& val, DeclaredType dtype) const {
    switch (dtype) {
        case DeclaredType::Int:
            return Value(static_cast<int64_t>(val.toInt()));
        case DeclaredType::Float:
            return Value(val.toFloat());
        case DeclaredType::Bool:
            return Value(val.isTruthy());
        case DeclaredType::String:
            return Value(val.toString());
        case DeclaredType::Rational:
            if (val.getType() == Value::Type::Int) {
                return Value(BigRational(static_cast<long>(val.getInt()), 1));
            }
            return val;
        case DeclaredType::BigInt:
            if (val.getType() == Value::Type::Int) {
                return Value(BigInt(static_cast<long>(val.getInt())));
            }
            return val;
        default:
            return val;
    }
}

std::pair<size_t, size_t> VM::getCacheStats() const {
    size_t total_hits = 0;
    size_t total_misses = 0;
    
    for (const auto& [_, cache] : member_cache_) {
        total_hits += cache.hits;
        total_misses += cache.misses;
    }
    
    return {total_hits, total_misses};
}

} // namespace rumina
