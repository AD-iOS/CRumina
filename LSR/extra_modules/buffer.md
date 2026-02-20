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
