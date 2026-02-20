# rumina:time

```lamina
include "rumina:time"
```

导入后可使用 `time` 对象：

- `time.now() -> Int`（Unix 毫秒时间戳）
- `time.sleep(ms: Int) -> Void`
- `time.startTimer() -> Timer`

`Timer` 对象实例方法：

- `timer.elapsedMs() -> Float`
- `timer.elapsedSec() -> Float`
