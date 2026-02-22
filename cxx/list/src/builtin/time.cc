#include <builtin/time.h>

#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace rumina {
namespace builtin {

static std::atomic<int64_t> timer_seq{1};
static std::mutex timers_mutex;
static std::unordered_map<int64_t, std::chrono::steady_clock::time_point> timers;
static std::chrono::steady_clock::time_point boot_time = std::chrono::steady_clock::now();

// 创建时间模块
Value create_time_module() {
    auto ns = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*ns)["now"] = Value::makeNativeFunction("time::now", time_now);
    (*ns)["hrtimeMs"] = Value::makeNativeFunction("time::hrtimeMs", time_hrtime_ms);
    (*ns)["sleep"] = Value::makeNativeFunction("time::sleep", time_sleep);
    (*ns)["startTimer"] = Value::makeNativeFunction("time::startTimer", time_start_timer);
    
    return Value::makeModule(ns);
}

// 构建Timer对象
static Value build_timer(int64_t id) {
    auto fields = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*fields)["__timer_id"] = Value(id);
    (*fields)["elapsedMs"] = Value::makeNativeFunction("Timer::elapsedMs", timer_elapsed_ms);
    (*fields)["elapsedSec"] = Value::makeNativeFunction("Timer::elapsedSec", timer_elapsed_sec);
    
    return Value::makeStruct(fields);
}

// 从Timer对象获取ID
static int64_t timer_id_from_self(const Value& value) {
    if (value.getType() != Value::Type::Struct) {
        throw std::runtime_error("Timer method expects Timer object");
    }
    
    auto fields = value.getStruct();
    auto it = fields->find("__timer_id");
    if (it == fields->end() || it->second.getType() != Value::Type::Int) {
        throw std::runtime_error("Invalid Timer object");
    }
    
    return it->second.getInt();
}

// 获取经过的时间
static std::chrono::duration<double> elapsed(int64_t id) {
    std::lock_guard<std::mutex> lock(timers_mutex);
    auto it = timers.find(id);
    if (it == timers.end()) {
        throw std::runtime_error("Timer has expired or is invalid");
    }
    return std::chrono::steady_clock::now() - it->second;
}

// time.now()
Value time_now(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("time.now expects no arguments");
    }
    
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    return Value(static_cast<int64_t>(millis));
}

// time.hrtimeMs()
Value time_hrtime_ms(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("time.hrtimeMs expects no arguments");
    }
    
    auto duration = std::chrono::steady_clock::now() - boot_time;
    double ms = std::chrono::duration<double, std::milli>(duration).count();
    return Value(ms);
}

// time.sleep(ms)
Value time_sleep(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("time.sleep expects 1 argument (ms)");
    }
    
    int64_t ms = args[0].toInt();
    if (ms < 0) {
        throw std::runtime_error("time.sleep expects non-negative milliseconds");
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return Value();
}

// time.startTimer()
Value time_start_timer(const std::vector<Value>& args) {
    if (!args.empty()) {
        throw std::runtime_error("time.startTimer expects no arguments");
    }
    
    int64_t id = timer_seq.fetch_add(1);
    
    {
        std::lock_guard<std::mutex> lock(timers_mutex);
        timers[id] = std::chrono::steady_clock::now();
    }
    
    return build_timer(id);
}

// timer.elapsedMs()
Value timer_elapsed_ms(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("timer.elapsedMs expects no arguments");
    }
    
    int64_t id = timer_id_from_self(args[0]);
    double ms = elapsed(id).count() * 1000.0;
    return Value(ms);
}

// timer.elapsedSec()
Value timer_elapsed_sec(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("timer.elapsedSec expects no arguments");
    }
    
    int64_t id = timer_id_from_self(args[0]);
    double sec = elapsed(id).count();
    return Value(sec);
}

} // namespace builtin
} // namespace rumina
