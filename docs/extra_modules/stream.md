# rumina:stream

```lamina
include "rumina:stream"
```

用于同步、拉取-阻塞式流模型。模块提供两类对象：`ReadStream` 与 `WriteStream`。

> 处理二进制数据时，通常配合 `include "rumina:buffer"`。

## 创建流

- `stream.openRead(path: String) -> ReadStream`
- `stream.openWrite(path: String, append: Bool) -> WriteStream`

`append=true` 时追加写入；`append=false` 时清空后从头写入。

## ReadStream 方法

- `readStream.readBytes(size: Int) -> Buffer | Null`
  - 读取最多 `size` 字节，EOF 时返回 `null`。
- `readStream.readUntil(delimiter: String | Buffer, maxBytes?: Int) -> Buffer | Null`
  - 持续读取直到遇到分隔符，返回分隔符之前的字节块（`Buffer`）。
  - 到达 EOF 且无数据时返回 `null`。
  - 可选 `maxBytes` 用于限制单次拉取长度。
- `readStream.readAll() -> Buffer`
  - 读取剩余全部字节直到 EOF。
- `readStream.seek(offset: Int) -> Int`
  - 将读游标移动到指定字节偏移，返回新偏移。
- `readStream.tell() -> Int`
  - 返回当前读游标字节偏移。
- `readStream.close() -> Void`
  - 关闭流并释放文件句柄。

## WriteStream 方法

- `writeStream.writeBytes(data: Buffer) -> Void`
- `writeStream.writeText(text: String) -> Void`
- `writeStream.flush() -> Void`
  - 强制将缓冲区数据刷入磁盘。
- `writeStream.seek(offset: Int) -> Int`
  - 将写游标移动到指定字节偏移，返回新偏移。
- `writeStream.tell() -> Int`
  - 返回当前写游标字节偏移。
- `writeStream.close() -> Void`
  - 关闭写流；关闭前会自动执行一次 flush。

## 用法示例（按换行分隔读取文本）

```lamina
include "rumina:stream";

var w = stream.openWrite("lines.txt", false);
w.writeText("line1\nline2\n");
w.flush();
w.close();

var r = stream.openRead("lines.txt");
print(r.readUntil("\n").toText()); // "line1"
print(r.readUntil("\n").toText()); // "line2"
print(r.readUntil("\n")); // null (EOF)
r.close();
```

## 用法示例（二进制）

```lamina
include "rumina:stream";
include "rumina:buffer";

var b = Buffer.alloc(3);
b.set(0, 65);
b.set(1, 66);
b.set(2, 67);

var w = stream.openWrite("data.bin", false);
w.writeBytes(b);
w.close();

var r = stream.openRead("data.bin");
var part = r.readBytes(2);
print(part.toText()); // "AB"
print(r.readBytes(999).toText()); // "C"
print(r.readBytes(1)); // null (EOF)
r.close();
```

## 注意事项

- 这是**同步阻塞**模型：读写调用会阻塞当前执行。
- `readBytes(size)` 中 `size` 应为非负整数。
- `readUntil` 返回 `Buffer`，文本请手动 `.toText()`。
- `readUntil` 的分隔符支持 `String` 或 `Buffer`，且不能为空。
- `seek/tell` 的单位是**字节偏移**（不是行号）。
- 流关闭后再读写会报错。
- 建议在使用完成后显式调用 `close()`，避免句柄占用。
