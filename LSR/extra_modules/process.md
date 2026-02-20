# rumina:process

```lamina
include "rumina:process"
```

导入后可使用 `process` 对象（进程控制与执行上下文）：

- `process.args() -> List<String>`
- `process.cwd() -> String`
- `process.setCwd(path: String) -> Void`
- `process.pid() -> Int`
- `process.exit(code: Int) -> Void`
