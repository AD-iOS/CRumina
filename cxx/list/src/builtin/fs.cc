#include <builtin/fs.h>
#include <builtin/buffer.h>

#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#endif

namespace rumina {
namespace builtin {
namespace fs {

// using namespace rumina::builtin::buffer;

namespace fs = std::filesystem;

// 创建文件系统模块
Value create_fs_module() {
    auto ns = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*ns)["readText"] = Value::makeNativeFunction("fs::readText", fs_read_text);
    (*ns)["readBytes"] = Value::makeNativeFunction("fs::readBytes", fs_read_bytes);
    (*ns)["writeText"] = Value::makeNativeFunction("fs::writeText", fs_write_text);
    (*ns)["writeBytes"] = Value::makeNativeFunction("fs::writeBytes", fs_write_bytes);
    (*ns)["append"] = Value::makeNativeFunction("fs::append", fs_append);
    
    (*ns)["exists"] = Value::makeNativeFunction("fs::exists", fs_exists);
    (*ns)["isFile"] = Value::makeNativeFunction("fs::isFile", fs_is_file);
    (*ns)["isDir"] = Value::makeNativeFunction("fs::isDir", fs_is_dir);
    (*ns)["stat"] = Value::makeNativeFunction("fs::stat", fs_stat);
    
    (*ns)["makeDir"] = Value::makeNativeFunction("fs::makeDir", fs_make_dir);
    (*ns)["makeDirAll"] = Value::makeNativeFunction("fs::makeDirAll", fs_make_dir_all);
    (*ns)["readDir"] = Value::makeNativeFunction("fs::readDir", fs_read_dir);
    (*ns)["remove"] = Value::makeNativeFunction("fs::remove", fs_remove);
    (*ns)["removeAll"] = Value::makeNativeFunction("fs::removeAll", fs_remove_all);
    (*ns)["rename"] = Value::makeNativeFunction("fs::rename", fs_rename);
    (*ns)["copy"] = Value::makeNativeFunction("fs::copy", fs_copy);
    (*ns)["realpath"] = Value::makeNativeFunction("fs::realpath", fs_realpath);
    (*ns)["readLink"] = Value::makeNativeFunction("fs::readLink", fs_read_link);
    (*ns)["link"] = Value::makeNativeFunction("fs::link", fs_link);
    (*ns)["symlink"] = Value::makeNativeFunction("fs::symlink", fs_symlink);
    (*ns)["chmod"] = Value::makeNativeFunction("fs::chmod", fs_chmod);
    
    return Value::makeModule(ns);
}

// 将参数转换为路径字符串
static std::string as_path(const Value& arg, const std::string& fn_name) {
    if (arg.getType() != Value::Type::String) {
        throw std::runtime_error(fn_name + " expects path as string");
    }
    return arg.getString();
}

// 检查参数数量
static void expect_arity(const std::vector<Value>& args, size_t expected, const std::string& sig) {
    if (args.size() != expected) {
        throw std::runtime_error(sig + " expects " + std::to_string(expected - 1) + " arguments");
    }
}

// 解析标志选项
static std::string parse_flag_opt(const Value& value, const std::string& fn_name, 
                                   const std::string& default_flag) {
    if (value.getType() == Value::Type::String) {
        return default_flag;
    } else if (value.getType() == Value::Type::Struct) {
        auto obj = value.getStruct();
        auto it = obj->find("flag");
        if (it != obj->end()) {
            if (it->second.getType() != Value::Type::String) {
                throw std::runtime_error(fn_name + " options.flag must be string");
            }
            return it->second.getString();
        }
        return default_flag;
    } else {
        throw std::runtime_error(fn_name + " options must be string or object struct");
    }
}

// 解析编码选项
static std::string parse_encoding_opt(const Value& value, const std::string& fn_name,
                                       const std::string& default_encoding) {
    if (value.getType() == Value::Type::String) {
        return value.getString();
    } else if (value.getType() == Value::Type::Struct) {
        auto obj = value.getStruct();
        auto it = obj->find("encoding");
        if (it != obj->end()) {
            if (it->second.getType() != Value::Type::String) {
                throw std::runtime_error(fn_name + " options.encoding must be string");
            }
            return it->second.getString();
        }
        return default_encoding;
    } else {
        throw std::runtime_error(fn_name + " options must be string or object struct");
    }
}

// 使用指定编码解码文本
static std::string decode_text_with_encoding(const std::vector<uint8_t>& bytes, 
                                              const std::string& encoding) {
    if (encoding == "utf8" || encoding == "utf-8") {
        return std::string(bytes.begin(), bytes.end());
    } else if (encoding == "hex") {
        std::string result;
        for (uint8_t b : bytes) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", b);
            result += hex;
        }
        return result;
    } else if (encoding == "base64" || encoding == "base64url") {
        static const char* b64_table = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        size_t i = 0;
        while (i < bytes.size()) {
            uint32_t octet_a = i < bytes.size() ? bytes[i++] : 0;
            uint32_t octet_b = i < bytes.size() ? bytes[i++] : 0;
            uint32_t octet_c = i < bytes.size() ? bytes[i++] : 0;
            
            uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
            
            result += b64_table[(triple >> 18) & 0x3F];
            result += b64_table[(triple >> 12) & 0x3F];
            result += b64_table[(triple >> 6) & 0x3F];
            result += b64_table[triple & 0x3F];
        }
        
        size_t padding = 3 - (bytes.size() % 3);
        if (padding < 3) {
            for (size_t j = 0; j < padding; ++j) {
                result[result.length() - 1 - j] = '=';
            }
        }
        
        if (encoding == "base64url") {
            for (char& c : result) {
                if (c == '+') c = '-';
                else if (c == '/') c = '_';
            }
        }
        
        return result;
    } else {
        throw std::runtime_error("Unsupported encoding: " + encoding);
    }
}

// 使用指定标志写入文件
static void write_with_flag(const std::string& path, const std::vector<uint8_t>& bytes,
                            const std::string& flag) {
    std::ios_base::openmode mode = std::ios::binary;
    
    if (flag == "w") {
        mode |= std::ios::trunc;
    } else if (flag == "a") {
        mode |= std::ios::app;
    } else {
        throw std::runtime_error("unsupported flag '" + flag + "' (supported: w, a)");
    }
    
    std::ofstream file(path, mode);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }
    
    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

// fs.readText(path, [options])
Value fs_read_text(const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        throw std::runtime_error("fs.readText(path, [options]) expects 1 or 2 arguments");
    }
    
    std::string path = as_path(args[0], "fs.readText");
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("fs.readText failed for '" + path + "'");
    }
    
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
    
    std::string encoding = "utf8";
    if (args.size() == 2) {
        encoding = parse_encoding_opt(args[1], "fs.readText", "utf8");
    }
    
    std::string content = decode_text_with_encoding(bytes, encoding);
    return Value(content);
}

// fs.readBytes(path, [options])
Value fs_read_bytes(const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        throw std::runtime_error("fs.readBytes(path, [options]) expects 1 or 2 arguments");
    }
    
    std::string path = as_path(args[0], "fs.readBytes");
    
    if (args.size() == 2) {
        parse_flag_opt(args[1], "fs.readBytes", "r");
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("fs.readBytes failed for '" + path + "'");
    }
    
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
    
    return buffer::new_buffer_from_bytes(bytes);
}

// fs.writeText(path, text, [options])
Value fs_write_text(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("fs.writeText(path, text, [options]) expects 2 or 3 arguments");
    }
    
    std::string path = as_path(args[0], "fs.writeText");
    
    if (args[1].getType() != Value::Type::String) {
        throw std::runtime_error("fs.writeText expects text as string");
    }
    const std::string& text = args[1].getString();
    
    std::vector<uint8_t> bytes(text.begin(), text.end());
    
    if (args.size() == 3) {
        std::string flag = parse_flag_opt(args[2], "fs.writeText", "w");
        write_with_flag(path, bytes, flag);
    } else {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("fs.writeText failed for '" + path + "'");
        }
        file.write(text.data(), text.size());
    }
    
    return Value();
}

// fs.writeBytes(path, data, [options])
Value fs_write_bytes(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("fs.writeBytes(path, data, [options]) expects 2 or 3 arguments");
    }
    
    std::string path = as_path(args[0], "fs.writeBytes");
    
    std::vector<uint8_t> bytes = buffer::buffer_to_bytes(args[1]);
    
    if (args.size() == 3) {
        std::string flag = parse_flag_opt(args[2], "fs.writeBytes", "w");
        write_with_flag(path, bytes, flag);
    } else {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("fs.writeBytes failed for '" + path + "'");
        }
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
    
    return Value();
}

// fs.append(path, data)
Value fs_append(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("fs.append(path, data) expects 2 arguments");
    }
    
    std::string path = as_path(args[0], "fs.append");
    
    std::vector<uint8_t> current;
    if (fs::exists(path)) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("fs.append read failed for '" + path + "'");
        }
        current.assign((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    }
    
    std::vector<uint8_t> append_data;
    if (args[1].getType() == Value::Type::String) {
        const std::string& s = args[1].getString();
        append_data.assign(s.begin(), s.end());
    } else {
        append_data = buffer::buffer_to_bytes(args[1]);
    }
    
    current.insert(current.end(), append_data.begin(), append_data.end());
    
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("fs.append write failed for '" + path + "'");
    }
    file.write(reinterpret_cast<const char*>(current.data()), current.size());
    
    return Value();
}

// fs.exists(path)
Value fs_exists(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.exists(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.exists");
    return Value(fs::exists(path));
}

// fs.isFile(path)
Value fs_is_file(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.isFile(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.isFile");
    return Value(fs::is_regular_file(path));
}

// fs.isDir(path)
Value fs_is_dir(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.isDir(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.isDir");
    return Value(fs::is_directory(path));
}

// fs.stat(path)
Value fs_stat(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.stat(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.stat");
    
    std::error_code ec;
    fs::file_status status = fs::status(path, ec);
    if (ec) {
        throw std::runtime_error("fs.stat failed for '" + path + "': " + ec.message());
    }

    uintmax_t file_size = 0;
    if (fs::is_regular_file(status)) {
        file_size = fs::file_size(path, ec);
        if (ec) {
            file_size = 0;
        }
    }

    int64_t modified_secs = 0;
    auto last_write_time = fs::last_write_time(path, ec);
    if (!ec) {
        auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            last_write_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        modified_secs = std::chrono::duration_cast<std::chrono::seconds>(
            system_time.time_since_epoch()).count();
    }
    
    // 创建结果结构体
    auto map = std::make_shared<std::unordered_map<std::string, Value>>();
    
    (*map)["size"] = Value(static_cast<int64_t>(file_size));
    (*map)["isFile"] = Value(fs::is_regular_file(status));
    (*map)["isDir"] = Value(fs::is_directory(status));
    (*map)["isSymlink"] = Value(fs::is_symlink(status));
    (*map)["modifiedTime"] = Value(modified_secs);
    
    return Value::makeStruct(map);
}

// fs.makeDir(path)
Value fs_make_dir(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.makeDir(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.makeDir");
    
    if (!fs::create_directory(path)) {
        throw std::runtime_error("fs.makeDir failed for '" + path + "'");
    }
    
    return Value();
}

// fs.makeDirAll(path)
Value fs_make_dir_all(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.makeDirAll(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.makeDirAll");
    
    if (!fs::create_directories(path)) {
        throw std::runtime_error("fs.makeDirAll failed for '" + path + "'");
    }
    
    return Value();
}

// fs.readDir(path, [withTypes])
Value fs_read_dir(const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        throw std::runtime_error("fs.readDir(path, [withTypes]) expects 1 or 2 arguments");
    }
    
    std::string path = as_path(args[0], "fs.readDir");
    
    bool with_types = false;
    if (args.size() == 2) {
        if (args[1].getType() != Value::Type::Bool) {
            throw std::runtime_error("fs.readDir withTypes must be bool");
        }
        with_types = args[1].getBool();
    }
    
    std::vector<Value> entries;
    
    for (const auto& entry : fs::directory_iterator(path)) {
        std::string name = entry.path().filename().string();
        
        if (with_types) {
            auto item = std::make_shared<std::unordered_map<std::string, Value>>();
            (*item)["name"] = Value(name);
            (*item)["isFile"] = Value(entry.is_regular_file());
            (*item)["isDir"] = Value(entry.is_directory());
            (*item)["isSymlink"] = Value(entry.is_symlink());
            entries.push_back(Value::makeStruct(item));
        } else {
            entries.push_back(Value(name));
        }
    }
    
    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(entries)));
}

// fs.remove(path)
Value fs_remove(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.remove(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.remove");
    
    if (!fs::remove(path)) {
        throw std::runtime_error("fs.remove failed for '" + path + "'");
    }
    
    return Value();
}

// fs.removeAll(path)
Value fs_remove_all(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.removeAll(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.removeAll");
    
    std::error_code ec;
    if (!fs::remove_all(path, ec)) {
        throw std::runtime_error("fs.removeAll failed for '" + path + "': " + ec.message());
    }
    
    return Value();
}

// fs.rename(oldPath, newPath)
Value fs_rename(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("fs.rename(oldPath, newPath) expects 2 arguments");
    }
    
    std::string old_path = as_path(args[0], "fs.rename");
    std::string new_path = as_path(args[1], "fs.rename");
    
    std::error_code ec;
    fs::rename(old_path, new_path, ec);
    if (ec) {
        throw std::runtime_error("fs.rename failed '" + old_path + "' -> '" + new_path + "': " + ec.message());
    }
    
    return Value();
}

// fs.copy(srcPath, destPath)
Value fs_copy(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("fs.copy(srcPath, destPath) expects 2 arguments");
    }
    
    std::string src = as_path(args[0], "fs.copy");
    std::string dst = as_path(args[1], "fs.copy");
    
    std::error_code ec;
    fs::copy(src, dst, ec);
    if (ec) {
        throw std::runtime_error("fs.copy failed '" + src + "' -> '" + dst + "': " + ec.message());
    }
    
    return Value();
}

// fs.realpath(path)
Value fs_realpath(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.realpath(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.realpath");
    
    std::error_code ec;
    fs::path canonical = fs::canonical(path, ec);
    if (ec) {
        throw std::runtime_error("fs.realpath failed for '" + path + "': " + ec.message());
    }
    
    return Value(canonical.string());
}

// fs.readLink(path)
Value fs_read_link(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("fs.readLink(path) expects 1 argument");
    }
    
    std::string path = as_path(args[0], "fs.readLink");
    
    std::error_code ec;
    fs::path target = fs::read_symlink(path, ec);
    if (ec) {
        throw std::runtime_error("fs.readLink failed for '" + path + "': " + ec.message());
    }
    
    return Value(target.string());
}

// fs.link(existingPath, newPath)
Value fs_link(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("fs.link(existingPath, newPath) expects 2 arguments");
    }
    
    std::string src = as_path(args[0], "fs.link");
    std::string dst = as_path(args[1], "fs.link");
    
    std::error_code ec;
    fs::create_hard_link(src, dst, ec);
    if (ec) {
        throw std::runtime_error("fs.link failed '" + src + "' -> '" + dst + "': " + ec.message());
    }
    
    return Value();
}

// fs.symlink(target, path)
Value fs_symlink(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("fs.symlink(target, path) expects 2 arguments");
    }
    
    std::string target = as_path(args[0], "fs.symlink");
    std::string path = as_path(args[1], "fs.symlink");
    
    std::error_code ec;
    fs::create_symlink(target, path, ec);
    if (ec) {
        throw std::runtime_error("fs.symlink failed '" + target + "' -> '" + path + "': " + ec.message());
    }
    
    return Value();
}

// fs.chmod(path, mode)
Value fs_chmod(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("fs.chmod(path, mode) expects 2 arguments");
    }
    
    std::string path = as_path(args[0], "fs.chmod");
    int64_t mode = args[1].toInt();
    if (mode < 0) {
        throw std::runtime_error("fs.chmod mode must be non-negative");
    }
    
#ifdef _WIN32
    (void)path;
    (void)mode;
    throw std::runtime_error("fs.chmod is not supported on this platform");
#else
    if (chmod(path.c_str(), static_cast<mode_t>(mode)) != 0) {
        throw std::runtime_error("fs.chmod failed for '" + path + "'");
    }
    return Value();
#endif
}

} // namespace fs
} // namespace builtin
} // namespace rumina
