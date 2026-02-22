#include <bytecode_optimizer.h>
#include <value_ops.h>

#include <algorithm>

namespace rumina {

struct Change {
    size_t index;
    OpCode new_op;
    size_t remove_count;
};

bool BytecodeOptimizer::optimize(ByteCode& bytecode) {
    modified_ = false;

    while (true) {
        bool before_modified = modified_;

        eliminateDeadPushPop(bytecode);
        eliminateRedundantDup(bytecode);
        mergeConstantOperations(bytecode);
        optimizeJumpChains(bytecode);
        eliminateNoopPatterns(bytecode);

        if (modified_ == before_modified) {
            break;
        }
    }

    return modified_;
}

void BytecodeOptimizer::eliminateDeadPushPop(ByteCode& bytecode) {
    auto& instructions = bytecode.getInstructions();
    auto& line_numbers = bytecode.getLineNumbers();
    
    std::vector<size_t> removals;
    
    for (size_t i = 0; i + 1 < instructions.size(); ++i) {
        const auto& current = instructions[i];
        const auto& next = instructions[i + 1];
        
        bool is_push = (current.type == OpCodeType::PushConst || 
                        current.type == OpCodeType::PushConstPooled);
        bool is_pop = (next.type == OpCodeType::Pop);
        
        if (is_push && is_pop) {
            removals.push_back(i);
            removals.push_back(i + 1);
            modified_ = true;
            i += 1;
        }
    }
    
    std::sort(removals.begin(), removals.end(), std::greater<size_t>());
    for (size_t idx : removals) {
        instructions.erase(instructions.begin() + idx);
        line_numbers.erase(line_numbers.begin() + idx);
    }
}

void BytecodeOptimizer::eliminateRedundantDup(ByteCode& bytecode) {
    auto& instructions = bytecode.getInstructions();
    auto& line_numbers = bytecode.getLineNumbers();
    
    std::vector<size_t> removals;
    
    for (size_t i = 0; i + 2 < instructions.size(); ++i) {
        const auto& first = instructions[i];
        const auto& second = instructions[i + 1];
        const auto& third = instructions[i + 2];
        
        if (first.type == OpCodeType::PushVar && 
            second.type == OpCodeType::Dup && 
            third.type == OpCodeType::PopVar) {
            
            if (first.payload_type == OpCode::PAYLOAD_STRING &&
                third.payload_type == OpCode::PAYLOAD_STRING) {
                
                const std::string& name1 = first.payload.str;
                const std::string& name2 = third.payload.str;
                
                if (name1 == name2) {
                    removals.push_back(i + 1);
                    removals.push_back(i + 2);
                    modified_ = true;
                    i += 2;
                }
            }
        }
    }
    
    std::sort(removals.begin(), removals.end(), std::greater<size_t>());
    for (size_t idx : removals) {
        instructions.erase(instructions.begin() + idx);
        line_numbers.erase(line_numbers.begin() + idx);
    }
}

void BytecodeOptimizer::mergeConstantOperations(ByteCode& bytecode) {
    auto& instructions = bytecode.getInstructions();
    auto& line_numbers = bytecode.getLineNumbers();
    auto& constants = bytecode.getConstants();
    
    std::vector<Change> changes;
    
    for (size_t i = 0; i + 2 < instructions.size(); ++i) {
        const auto& first = instructions[i];
        const auto& second = instructions[i + 1];
        const auto& third = instructions[i + 2];
        
        if (first.type == OpCodeType::PushConstPooled &&
            second.type == OpCodeType::PushConstPooled &&
            first.payload_type == OpCode::PAYLOAD_SIZE &&
            second.payload_type == OpCode::PAYLOAD_SIZE) {
            
            size_t idx1 = first.payload.size;
            size_t idx2 = second.payload.size;
            
            if (idx1 < constants.size() && idx2 < constants.size()) {
                const Value& val1 = constants[idx1];
                const Value& val2 = constants[idx2];
                
                if (val1.getType() == Value::Type::Int && 
                    val2.getType() == Value::Type::Int) {
                    
                    int64_t a = val1.getInt();
                    int64_t b = val2.getInt();
                    int64_t result = 0;
                    bool can_merge = false;
                    
                    if (third.type == OpCodeType::Add) {
                        result = a + b;
                        can_merge = true;
                    } else if (third.type == OpCodeType::Sub) {
                        result = a - b;
                        can_merge = true;
                    } else if (third.type == OpCodeType::Mul) {
                        result = a * b;
                        can_merge = true;
                    }
                    
                    if (can_merge) {
                        changes.push_back({i, OpCode(OpCodeType::PushConst, Value(static_cast<int64_t>(result))), 3});
                        modified_ = true;
                        i += 2;
                    }
                }
            }
        }
    }
    
    for (auto it = changes.rbegin(); it != changes.rend(); ++it) {
        instructions[it->index] = it->new_op;
        for (size_t j = 1; j < it->remove_count; ++j) {
            instructions.erase(instructions.begin() + it->index + 1);
            line_numbers.erase(line_numbers.begin() + it->index + 1);
        }
    }
}

void BytecodeOptimizer::optimizeJumpChains(ByteCode& bytecode) {
    auto& instructions = bytecode.getInstructions();
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto& op = instructions[i];
        
        if ((op.type == OpCodeType::Jump ||
             op.type == OpCodeType::JumpIfFalse ||
             op.type == OpCodeType::JumpIfTrue) &&
            op.payload_type == OpCode::PAYLOAD_SIZE) {
            
            size_t target = op.payload.size;
            std::unordered_set<size_t> visited;
            size_t final_target = target;
            
            while (final_target < instructions.size() && 
                   visited.find(final_target) == visited.end()) {
                visited.insert(final_target);
                
                const auto& target_op = instructions[final_target];
                if (target_op.type == OpCodeType::Jump &&
                    target_op.payload_type == OpCode::PAYLOAD_SIZE) {
                    final_target = target_op.payload.size;
                    modified_ = true;
                } else {
                    break;
                }
            }
            
            if (final_target != target) {
                op.payload.size = final_target;
            }
        }
    }
}

void BytecodeOptimizer::eliminateNoopPatterns(ByteCode& bytecode) {
    auto& instructions = bytecode.getInstructions();
    auto& line_numbers = bytecode.getLineNumbers();
    
    std::vector<size_t> removals;
    
    for (size_t i = 0; i + 1 < instructions.size(); ++i) {
        const auto& current = instructions[i];
        const auto& next = instructions[i + 1];
        
        if (current.type == OpCodeType::PushVar && 
            next.type == OpCodeType::PopVar &&
            current.payload_type == OpCode::PAYLOAD_STRING &&
            next.payload_type == OpCode::PAYLOAD_STRING) {
            
            const std::string& name1 = current.payload.str;
            const std::string& name2 = next.payload.str;
            
            if (name1 == name2) {
                removals.push_back(i);
                removals.push_back(i + 1);
                modified_ = true;
                i += 1;
            }
        }
    }
    
    std::sort(removals.begin(), removals.end(), std::greater<size_t>());
    for (size_t idx : removals) {
        instructions.erase(instructions.begin() + idx);
        line_numbers.erase(line_numbers.begin() + idx);
    }
}

} // namespace rumina
