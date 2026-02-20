# rumina:buffer

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

## 用法示例

```lamina
include "rumina:buffer";

var b = Buffer.alloc(5);
b.set(0, 72);
b.set(1, 101);
b.set(2, 108);
b.set(3, 108);
b.set(4, 111);

print(b.length());   // 5
print(b.get(0));     // 72
print(b.toText());   // "Hello"

var sub = b.slice(1, 4);
print(sub.toText()); // "ell"
```

## 注意事项

- `get/set` 的索引必须在范围内。
- `set` 的字节值必须是 `0..255`。
- `toText()` 需要字节序列是合法 UTF-8，否则会报错。
