# rumina:buffer

```lamina
include "rumina:buffer"
```

二进制字节容器。每个字节范围为 `0..255`。

## 构造器 / 命名空间方法

- `Buffer.alloc(size: Int) -> Buffer`
  分配固定长度 Buffer，初始值全 0。
- `Buffer.from(data: String | List<Int> | Buffer, encoding?: String) -> Buffer`
  从字符串、字节数组或已有 Buffer 构造。`encoding` 支持 `utf8`/`hex`/`base64`/`base64url`。
- `Buffer.concat(buffers: List<Buffer>) -> Buffer`
  按顺序拼接多个 Buffer。

## 实例方法

- `length() -> Int`
  返回字节长度。
- `get(index: Int) -> Int`
  读取指定位置字节。
- `set(index: Int, value: Int) -> Void`
  设置指定位置字节。
- `slice(start: Int, end: Int) -> Buffer`
  返回 `[start, end)` 的新 Buffer。
- `subarray(start: Int, end?: Int) -> Buffer`
  支持负索引的切片版本（按字节）。
- `toText() -> String`
  按 UTF-8 解码。
- `toHex() -> String`
  导出十六进制字符串。
- `toBase64() -> String`
  导出 base64 字符串（含 padding）。
- `toBase64Url() -> String`
  导出 base64url 字符串（无 padding）。
- `copy(target: Buffer, targetStart?: Int, sourceStart?: Int, sourceEnd?: Int) -> Int`
  复制字节到目标 Buffer，返回实际复制长度。
- `fill(value: Int, start?: Int, end?: Int) -> Void`
  用单字节值填充区间。
- `indexOf(pattern: Int | String | Buffer, offset?: Int, encoding?: String) -> Int`
  查找首次出现位置，未找到返回 `-1`。支持负偏移。
- `includes(pattern: Int | String | Buffer, offset?: Int, encoding?: String) -> Bool`
  是否包含目标片段。
- `equals(other: Buffer) -> Bool`
  字节级完全相等比较。
- `compare(other: Buffer) -> Int`
  字典序比较：`-1/0/1`。

## 注意事项

- 索引越界会报错。
- `set/fill` 值必须为 `0..255`。
- `toText()` 仅适用于有效 UTF-8。
