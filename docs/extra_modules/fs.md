# rumina:fs

```lamina
include "rumina:fs"
```

导入后可使用 `fs` 对象：`fs.xxx(...)`。

### 文件读写

- `fs.readText(path: String) -> String`
- `fs.readBytes(path: String) -> Buffer`
- `fs.writeText(path: String, text: String) -> Void`
- `fs.writeBytes(path: String, data: Buffer) -> Void`
- `fs.append(path: String, data: String | Buffer) -> Void`

### 状态与检查

- `fs.exists(path: String) -> Bool`
- `fs.isFile(path: String) -> Bool`
- `fs.isDir(path: String) -> Bool`
- `fs.stat(path: String) -> Dict`
  - 返回字段：`size`, `isFile`, `isDir`, `modifiedTime`

### 实体操作

- `fs.makeDir(path: String) -> Void`
- `fs.makeDirAll(path: String) -> Void`
- `fs.readDir(path: String) -> List<String>`
- `fs.remove(path: String) -> Void`
- `fs.removeAll(path: String) -> Void`
- `fs.rename(oldPath: String, newPath: String) -> Void`
- `fs.copy(srcPath: String, destPath: String) -> Void`

## 用法示例

```lamina
include "rumina:buffer";
include "rumina:fs";

fs.writeText("notes.txt", "hello");
fs.append("notes.txt", " world");
print(fs.readText("notes.txt")); // "hello world"

var b = Buffer.alloc(3);
b.set(0, 65);
b.set(1, 66);
b.set(2, 67);
fs.writeBytes("data.bin", b);

var rb = fs.readBytes("data.bin");
print(rb.get(0));    // 65
print(rb.toText());  // "ABC"

print(fs.exists("notes.txt"));
print(fs.isFile("notes.txt"));

var st = fs.stat("notes.txt");
print(st.size);
print(st.modifiedTime);

fs.makeDirAll("tmp/a/b");
print(fs.readDir("tmp"));

fs.copy("notes.txt", "notes.bak.txt");
fs.rename("notes.bak.txt", "notes2.txt");
fs.remove("notes2.txt");
fs.removeAll("tmp");
```

## 注意事项

- `writeText/writeBytes` 为覆盖写入。
- `remove` 仅删除文件或空目录；非空目录请用 `removeAll`。
- `append` 仅支持 `String` 或 `Buffer`。
