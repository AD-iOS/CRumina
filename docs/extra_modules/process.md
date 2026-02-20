# rumina:process

```lamina
include "rumina:process"
```

进程与执行上下文信息。

## 方法

- `process.args() -> List<String>`
  启动参数列表。
- `process.cwd() -> String`
  当前工作目录。
- `process.setCwd(path: String) -> Void`
  修改当前工作目录。
- `process.pid() -> Int`
  当前进程 ID。
- `process.exit(code: Int) -> Void`
  立即退出进程。
- `process.platform() -> String`
  运行平台标识。
- `process.arch() -> String`
  CPU 架构标识。
- `process.version() -> String`
  Rumina 运行时版本字符串。
- `process.execPath() -> String`
  当前可执行文件路径。
