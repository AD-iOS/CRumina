# rumina:time

```lamina
include "rumina:time"
```

同步时间与计时 API。

## time 方法

- `time.now() -> Int`
  Unix 毫秒时间戳（系统时钟）。
- `time.hrtimeMs() -> Float`
  单调时钟毫秒值（适合性能计时）。
- `time.sleep(ms: Int) -> Void`
  阻塞休眠指定毫秒。
- `time.startTimer() -> Timer`
  创建高精度计时器。

## Timer 方法

- `timer.elapsedMs() -> Float`
  已耗时毫秒。
- `timer.elapsedSec() -> Float`
  已耗时秒。
