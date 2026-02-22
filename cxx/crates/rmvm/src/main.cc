#include <vm.h>
#include <interpreter.h>
#include <iostream>
#include <fstream>
#include <thread>

namespace rumina {

constexpr size_t STACK_SIZE = 128 * 1024 * 1024; // 128 MB

int run_bytecode_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error reading file '" << filename << "'\n";
        return 1;
    }
    
    std::string contents((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    
    try {
        ByteCode bytecode = ByteCode::deserialize(contents);
        
        Interpreter interpreter;
        auto globals = interpreter.getGlobals();
        VM vm(globals);
        vm.load(std::move(bytecode));
        
        auto result = vm.run();
        if (result.is_error()) {
            std::cerr << result.error() << "\n";
            return 1;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace rumina

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: rmvm <file.rmc>\n";
        std::cerr << "  Execute Rumina bytecode file\n";
        return 1;
    }
    
    std::string filename = argv[1];
    
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".rmc") {
        std::cerr << "Error: File must have .rmc extension\n";
        return 1;
    }
    
    // 在新线程中运行以增加栈大小
    std::thread t([&]() {
        rumina::run_bytecode_file(filename);
    });
    
    t.join();
    return 0;
}
