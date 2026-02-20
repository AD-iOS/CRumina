# Rumina 特殊模块与方法

Rumina 通过 `include "rumina:*"` 导入 Rumina 特有模块。

## 1 `rumina:buffer`

```lamina
include "rumina:buffer"
```

导入后可使用 `Buffer` 命名空间与 Buffer 实例方法。

### 构造

- `Buffer.alloc(size: Int) -> Buffer`

### Buffer 实例方法

- `length() -> Int`
- `get(index: Int) -> Int`
- `set(index: Int, value: Int) -> Void`
- `slice(start: Int, end: Int) -> Buffer`
- `toText() -> String`

---

## 2 `rumina:fs`

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
