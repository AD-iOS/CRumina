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

## 用法示例

```lamina
include "rumina:process";

var args = process.args();
print(args);

print(process.pid());

var old = process.cwd();
process.setCwd("./examples");
print(process.cwd());
process.setCwd(old);

// process.exit(0); // 立即退出进程
```

## 注意事项

- `args()[0]` 通常是解释器或脚本入口信息。
- `process.exit(code)` 调用后不会继续执行后续语句。
