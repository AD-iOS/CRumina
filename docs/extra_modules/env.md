# rumina:env

```lamina
include "rumina:env"
```

导入后可使用 `env` 对象（仅环境变量管理）：

- `env.get(key: String) -> String | Null`
- `env.set(key: String, value: String) -> Void`
- `env.has(key: String) -> Bool`
- `env.remove(key: String) -> Void`
- `env.all() -> Dict`

## 用法示例

```lamina
include "rumina:env";

print(env.get("HOME"));

env.set("RUMINA_MODE", "dev");
print(env.has("RUMINA_MODE"));
print(env.get("RUMINA_MODE")); // dev

var all = env.all();
print(all.RUMINA_MODE);

env.remove("RUMINA_MODE");
print(env.get("RUMINA_MODE")); // null
```

## 注意事项

- `env.set/remove` 只影响当前 Rumina 进程及其子进程。
- 不会修改操作系统的永久环境变量配置。
