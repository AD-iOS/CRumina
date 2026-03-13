#include <builtin/random.h>

#include <random>

namespace rumina {
namespace builtin {
namespace random_ns {
    static std::mt19937& get_rng() {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        return rng;
    }

Value rand(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("random::rand expects 0 arguments");
    }
    
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return Value(dist(random_ns::get_rng()));
}

Value randint(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("random::randint expects 2 arguments (start, end)");
    }
    
    int64_t start = args[0].toInt();
    int64_t end = args[1].toInt();
    
    std::uniform_int_distribution<int64_t> dist(start, end);
    return Value(dist(random_ns::get_rng()));
}

Value random(const std::vector<Value>& args) {
    return rand(args);
}

} // namespace random
} // namespace builtin
} // namespace rumina
