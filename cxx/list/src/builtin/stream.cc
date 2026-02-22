#include <builtin/stream.h>
#include <builtin/buffer.h>

#include <fstream>
#include <mutex>
#include <unordered_map>
#include <atomic>

namespace rumina {
namespace builtin {
namespace stream {

// using namespace rumina::builtin::buffer;

static std::atomic<int64_t> read_stream_seq{1};
static std::atomic<int64_t> write_stream_seq{1};
static std::mutex readers_mutex;
static std::mutex writers_mutex;
static std::unordered_map<int64_t, std::shared_ptr<std::ifstream>> readers;
static std::unordered_map<int64_t, std::shared_ptr<std::ofstream>> writers;

constexpr const char* READ_STREAM_ID_KEY = "__read_stream_id";
constexpr const char* WRITE_STREAM_ID_KEY = "__write_stream_id";

// 创建流模块
Value create_stream_module() {
    auto ns = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*ns)["openRead"] = Value::makeNativeFunction("stream::openRead", stream_open_read);
    (*ns)["openWrite"] = Value::makeNativeFunction("stream::openWrite", stream_open_write);
    
    return Value::makeModule(ns);
}

// 创建方法
static Value method(const std::string& name, NativeFunction func) {
    return Value::makeNativeFunction(name, func);
}

// 获取字符串参数
static std::string as_string(const Value& v, const std::string& fn_name) {
    if (v.getType() != Value::Type::String) {
        throw std::runtime_error(fn_name + " expects string argument");
    }
    return v.getString();
}

// 获取布尔参数
static bool as_bool(const Value& v, const std::string& fn_name) {
    if (v.getType() != Value::Type::Bool) {
        throw std::runtime_error(fn_name + " expects bool argument");
    }
    return v.getBool();
}

// 从ReadStream对象获取ID
static int64_t read_stream_id_from_self(const Value& value) {
    if (value.getType() != Value::Type::Struct) {
        throw std::runtime_error("ReadStream method expects ReadStream object");
    }
    
    auto fields = value.getStruct();
    auto it = fields->find(READ_STREAM_ID_KEY);
    if (it == fields->end() || it->second.getType() != Value::Type::Int) {
        throw std::runtime_error("Invalid ReadStream object");
    }
    
    return it->second.getInt();
}

// 从WriteStream对象获取ID
static int64_t write_stream_id_from_self(const Value& value) {
    if (value.getType() != Value::Type::Struct) {
        throw std::runtime_error("WriteStream method expects WriteStream object");
    }
    
    auto fields = value.getStruct();
    auto it = fields->find(WRITE_STREAM_ID_KEY);
    if (it == fields->end() || it->second.getType() != Value::Type::Int) {
        throw std::runtime_error("Invalid WriteStream object");
    }
    
    return it->second.getInt();
}

// 构建ReadStream对象
static Value build_read_stream(int64_t id) {
    auto fields = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*fields)[READ_STREAM_ID_KEY] = Value(id);
    (*fields)["readBytes"] = method("ReadStream::readBytes", read_stream_read_bytes);
    (*fields)["readUntil"] = method("ReadStream::readUntil", read_stream_read_until);
    (*fields)["readAll"] = method("ReadStream::readAll", read_stream_read_all);
    (*fields)["seek"] = method("ReadStream::seek", read_stream_seek);
    (*fields)["tell"] = method("ReadStream::tell", read_stream_tell);
    (*fields)["isClosed"] = method("ReadStream::isClosed", read_stream_is_closed);
    (*fields)["close"] = method("ReadStream::close", read_stream_close);
    
    return Value::makeStruct(fields);
}

// 构建WriteStream对象
static Value build_write_stream(int64_t id) {
    auto fields = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*fields)[WRITE_STREAM_ID_KEY] = Value(id);
    (*fields)["writeBytes"] = method("WriteStream::writeBytes", write_stream_write_bytes);
    (*fields)["writeText"] = method("WriteStream::writeText", write_stream_write_text);
    (*fields)["flush"] = method("WriteStream::flush", write_stream_flush);
    (*fields)["seek"] = method("WriteStream::seek", write_stream_seek);
    (*fields)["tell"] = method("WriteStream::tell", write_stream_tell);
    (*fields)["isClosed"] = method("WriteStream::isClosed", write_stream_is_closed);
    (*fields)["close"] = method("WriteStream::close", write_stream_close);
    
    return Value::makeStruct(fields);
}

// stream.openRead(path)
Value stream_open_read(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("stream.openRead expects 1 argument (path)");
    }
    
    std::string path = as_string(args[0], "stream.openRead");
    
    auto file = std::make_shared<std::ifstream>(path, std::ios::binary);
    if (!file->is_open()) {
        throw std::runtime_error("stream.openRead failed for '" + path + "'");
    }
    
    int64_t id = read_stream_seq.fetch_add(1);
    
    {
        std::lock_guard<std::mutex> lock(readers_mutex);
        readers[id] = file;
    }
    
    return build_read_stream(id);
}

// stream.openWrite(path, append)
Value stream_open_write(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("stream.openWrite expects 2 arguments (path, append)");
    }
    
    std::string path = as_string(args[0], "stream.openWrite");
    bool append = as_bool(args[1], "stream.openWrite");
    
    std::ios_base::openmode mode = std::ios::binary;
    if (append) {
        mode |= std::ios::app;
    } else {
        mode |= std::ios::trunc;
    }
    
    auto file = std::make_shared<std::ofstream>(path, mode);
    if (!file->is_open()) {
        throw std::runtime_error("stream.openWrite failed for '" + path + "'");
    }
    
    int64_t id = write_stream_seq.fetch_add(1);
    
    {
        std::lock_guard<std::mutex> lock(writers_mutex);
        writers[id] = file;
    }
    
    return build_write_stream(id);
}

// 获取分隔符字节
static std::vector<uint8_t> delimiter_bytes(const Value& value) {
    if (value.getType() == Value::Type::String) {
        const std::string& s = value.getString();
        return std::vector<uint8_t>(s.begin(), s.end());
    } else if (value.getType() == Value::Type::Struct) {
        return buffer::buffer_to_bytes(value);
    } else {
        throw std::runtime_error("readStream.readUntil expects delimiter as String or Buffer");
    }
}

// readStream.readBytes(size)
Value read_stream_read_bytes(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("readStream.readBytes expects 1 argument (size)");
    }
    
    int64_t id = read_stream_id_from_self(args[0]);
    int64_t size = args[1].toInt();
    if (size < 0) {
        throw std::runtime_error("readStream.readBytes expects non-negative size");
    }
    
    std::shared_ptr<std::ifstream> file;
    {
        std::lock_guard<std::mutex> lock(readers_mutex);
        auto it = readers.find(id);
        if (it == readers.end()) {
            throw std::runtime_error("ReadStream is closed or invalid");
        }
        file = it->second;
    }
    
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    file->read(reinterpret_cast<char*>(buf.data()), size);
    size_t n = file->gcount();
    
    if (n == 0) {
        return Value();
    }
    
    buf.resize(n);
    return buffer::new_buffer_from_bytes(buf);
}

// readStream.readUntil(delimiter, maxBytes?)
Value read_stream_read_until(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("readStream.readUntil expects 1 or 2 arguments (delimiter, maxBytes?)");
    }
    
    int64_t id = read_stream_id_from_self(args[0]);
    std::vector<uint8_t> delim = delimiter_bytes(args[1]);
    
    if (delim.empty()) {
        throw std::runtime_error("readStream.readUntil delimiter cannot be empty");
    }
    
    std::optional<size_t> max_bytes;
    if (args.size() == 3) {
        int64_t mb = args[2].toInt();
        if (mb < 0) {
            throw std::runtime_error("readStream.readUntil maxBytes must be non-negative");
        }
        max_bytes = static_cast<size_t>(mb);
    }
    
    std::shared_ptr<std::ifstream> file;
    {
        std::lock_guard<std::mutex> lock(readers_mutex);
        auto it = readers.find(id);
        if (it == readers.end()) {
            throw std::runtime_error("ReadStream is closed or invalid");
        }
        file = it->second;
    }
    
    std::vector<uint8_t> out;
    bool reached_eof = false;
    
    while (true) {
        if (max_bytes.has_value() && out.size() >= max_bytes.value()) {
            break;
        }
        
        char c;
        if (!file->get(c)) {
            reached_eof = true;
            break;
        }
        
        out.push_back(static_cast<uint8_t>(c));
        
        if (out.size() >= delim.size()) {
            bool match = true;
            for (size_t i = 0; i < delim.size(); ++i) {
                if (out[out.size() - delim.size() + i] != delim[i]) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                out.resize(out.size() - delim.size());
                break;
            }
        }
    }
    
    if (out.empty() && reached_eof) {
        return Value();
    }
    
    return buffer::new_buffer_from_bytes(out);
}

// readStream.readAll()
Value read_stream_read_all(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("readStream.readAll expects no arguments");
    }
    
    int64_t id = read_stream_id_from_self(args[0]);
    
    std::shared_ptr<std::ifstream> file;
    {
        std::lock_guard<std::mutex> lock(readers_mutex);
        auto it = readers.find(id);
        if (it == readers.end()) {
            throw std::runtime_error("ReadStream is closed or invalid");
        }
        file = it->second;
    }
    
    std::vector<uint8_t> out;
    char c;
    while (file->get(c)) {
        out.push_back(static_cast<uint8_t>(c));
    }
    
    return buffer::new_buffer_from_bytes(out);
}

// readStream.seek(offset)
Value read_stream_seek(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("readStream.seek expects 1 argument (offset)");
    }
    
    int64_t id = read_stream_id_from_self(args[0]);
    int64_t offset = args[1].toInt();
    if (offset < 0) {
        throw std::runtime_error("readStream.seek expects non-negative offset");
    }
    
    std::shared_ptr<std::ifstream> file;
    {
        std::lock_guard<std::mutex> lock(readers_mutex);
        auto it = readers.find(id);
        if (it == readers.end()) {
            throw std::runtime_error("ReadStream is closed or invalid");
        }
        file = it->second;
    }
    
    file->seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (file->fail()) {
        throw std::runtime_error("readStream.seek failed");
    }
    
    std::streampos pos = file->tellg();
    return Value(static_cast<int64_t>(pos));
}

// readStream.tell()
Value read_stream_tell(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("readStream.tell expects no arguments");
    }
    
    int64_t id = read_stream_id_from_self(args[0]);
    
    std::shared_ptr<std::ifstream> file;
    {
        std::lock_guard<std::mutex> lock(readers_mutex);
        auto it = readers.find(id);
        if (it == readers.end()) {
            throw std::runtime_error("ReadStream is closed or invalid");
        }
        file = it->second;
    }
    
    std::streampos pos = file->tellg();
    return Value(static_cast<int64_t>(pos));
}

// readStream.isClosed()
Value read_stream_is_closed(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("readStream.isClosed expects no arguments");
    }
    
    int64_t id = read_stream_id_from_self(args[0]);
    
    std::lock_guard<std::mutex> lock(readers_mutex);
    return Value(readers.find(id) == readers.end());
}

// readStream.close()
Value read_stream_close(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("readStream.close expects no arguments");
    }
    
    int64_t id = read_stream_id_from_self(args[0]);
    
    std::lock_guard<std::mutex> lock(readers_mutex);
    readers.erase(id);
    
    return Value();
}

// writeStream.writeBytes(data)
Value write_stream_write_bytes(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("writeStream.writeBytes expects 1 argument (data)");
    }
    
    int64_t id = write_stream_id_from_self(args[0]);
    std::vector<uint8_t> bytes = buffer::buffer_to_bytes(args[1]);
    
    std::shared_ptr<std::ofstream> file;
    {
        std::lock_guard<std::mutex> lock(writers_mutex);
        auto it = writers.find(id);
        if (it == writers.end()) {
            throw std::runtime_error("WriteStream is closed or invalid");
        }
        file = it->second;
    }
    
    file->write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    if (file->fail()) {
        throw std::runtime_error("writeStream.writeBytes failed");
    }
    
    return Value();
}

// writeStream.writeText(text)
Value write_stream_write_text(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("writeStream.writeText expects 1 argument (text)");
    }
    
    int64_t id = write_stream_id_from_self(args[0]);
    std::string text = as_string(args[1], "writeStream.writeText");
    
    std::shared_ptr<std::ofstream> file;
    {
        std::lock_guard<std::mutex> lock(writers_mutex);
        auto it = writers.find(id);
        if (it == writers.end()) {
            throw std::runtime_error("WriteStream is closed or invalid");
        }
        file = it->second;
    }
    
    file->write(text.data(), text.size());
    if (file->fail()) {
        throw std::runtime_error("writeStream.writeText failed");
    }
    
    return Value();
}

// writeStream.flush()
Value write_stream_flush(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("writeStream.flush expects no arguments");
    }
    
    int64_t id = write_stream_id_from_self(args[0]);
    
    std::shared_ptr<std::ofstream> file;
    {
        std::lock_guard<std::mutex> lock(writers_mutex);
        auto it = writers.find(id);
        if (it == writers.end()) {
            throw std::runtime_error("WriteStream is closed or invalid");
        }
        file = it->second;
    }
    
    file->flush();
    if (file->fail()) {
        throw std::runtime_error("writeStream.flush failed");
    }
    
    return Value();
}

// writeStream.seek(offset)
Value write_stream_seek(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("writeStream.seek expects 1 argument (offset)");
    }
    
    int64_t id = write_stream_id_from_self(args[0]);
    int64_t offset = args[1].toInt();
    if (offset < 0) {
        throw std::runtime_error("writeStream.seek expects non-negative offset");
    }
    
    std::shared_ptr<std::ofstream> file;
    {
        std::lock_guard<std::mutex> lock(writers_mutex);
        auto it = writers.find(id);
        if (it == writers.end()) {
            throw std::runtime_error("WriteStream is closed or invalid");
        }
        file = it->second;
    }
    
    file->seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    if (file->fail()) {
        throw std::runtime_error("writeStream.seek failed");
    }
    
    std::streampos pos = file->tellp();
    return Value(static_cast<int64_t>(pos));
}

// writeStream.tell()
Value write_stream_tell(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("writeStream.tell expects no arguments");
    }
    
    int64_t id = write_stream_id_from_self(args[0]);
    
    std::shared_ptr<std::ofstream> file;
    {
        std::lock_guard<std::mutex> lock(writers_mutex);
        auto it = writers.find(id);
        if (it == writers.end()) {
            throw std::runtime_error("WriteStream is closed or invalid");
        }
        file = it->second;
    }
    
    std::streampos pos = file->tellp();
    return Value(static_cast<int64_t>(pos));
}

// writeStream.isClosed()
Value write_stream_is_closed(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("writeStream.isClosed expects no arguments");
    }
    
    int64_t id = write_stream_id_from_self(args[0]);
    
    std::lock_guard<std::mutex> lock(writers_mutex);
    return Value(writers.find(id) == writers.end());
}

// writeStream.close()
Value write_stream_close(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("writeStream.close expects no arguments");
    }
    
    int64_t id = write_stream_id_from_self(args[0]);
    
    std::lock_guard<std::mutex> lock(writers_mutex);
    auto it = writers.find(id);
    if (it != writers.end()) {
        it->second->flush();
        writers.erase(it);
    }
    
    return Value();
}

} // namespace stream
} // namespace builtin
} // namespace rumina
