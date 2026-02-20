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

## 用法示例

```lamina
include "rumina:time";

var t0 = time.now();
time.sleep(50);
var t1 = time.now();
print(t1 - t0);

var timer = time.startTimer();
// 执行需要测量的逻辑...
time.sleep(20);
print(timer.elapsedMs());
print(timer.elapsedSec());
```

## 注意事项

- `time.now()` 是 Unix 毫秒时间戳（系统时钟）。
- 性能计时推荐 `startTimer()` + `elapsedMs/elapsedSec`（单调时钟）。
