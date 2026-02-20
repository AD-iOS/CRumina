# rumina:env

```lamina
include "rumina:env"
```

环境变量管理（仅当前进程及其子进程）。

## 方法

- `env.get(key: String) -> String | Null`
  获取环境变量值。
- `env.set(key: String, value: String) -> Void`
  设置环境变量。
- `env.has(key: String) -> Bool`
  判断变量是否存在。
- `env.remove(key: String) -> Void`
  删除变量。
- `env.all() -> Dict`
  返回全部键值。
- `env.keys() -> List<String>`
  返回全部键名。
