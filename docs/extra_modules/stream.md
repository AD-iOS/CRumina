# rumina:stream

```lamina
include "rumina:stream"
```

同步、拉取-阻塞式文件流模型。

## 构造

- `stream.openRead(path: String) -> ReadStream`
  打开只读流。
- `stream.openWrite(path: String, append: Bool) -> WriteStream`
  打开写流；`append=true` 追加，`false` 覆盖。

## ReadStream 方法

- `readBytes(size: Int) -> Buffer | Null`
  读取最多 `size` 字节，EOF 返回 `null`。
- `readUntil(delimiter: String | Buffer, maxBytes?: Int) -> Buffer | Null`
  读取到分隔符为止（不含分隔符），EOF 无数据返回 `null`。
- `readAll() -> Buffer`
  读取剩余全部字节。
- `seek(offset: Int) -> Int`
  设置读游标到字节偏移。
- `tell() -> Int`
  获取当前读游标偏移。
- `isClosed() -> Bool`
  判断流是否已关闭。
- `close() -> Void`
  关闭读流。

## WriteStream 方法

- `writeBytes(data: Buffer) -> Void`
  写入字节块。
- `writeText(text: String) -> Void`
  写入文本。
- `flush() -> Void`
  强制刷新缓冲到磁盘。
- `seek(offset: Int) -> Int`
  设置写游标到字节偏移。
- `tell() -> Int`
  获取当前写游标偏移。
- `isClosed() -> Bool`
  判断流是否已关闭。
- `close() -> Void`
  关闭写流（会自动 flush）。
