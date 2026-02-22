#pragma once

#include "vm.h"
#include <vector>
#include <unordered_set>

namespace rumina {

// 字节码窥孔优化器
class BytecodeOptimizer {
public:
    BytecodeOptimizer() : modified_(false) {}

    // 优化字节码，返回true如果有任何优化被应用
    bool optimize(ByteCode& bytecode);

private:
    bool modified_;

    void eliminateDeadPushPop(ByteCode& bytecode);
    void eliminateRedundantDup(ByteCode& bytecode);
    void mergeConstantOperations(ByteCode& bytecode);
    void optimizeJumpChains(ByteCode& bytecode);
    void eliminateNoopPatterns(ByteCode& bytecode);
};

} // namespace rumina
