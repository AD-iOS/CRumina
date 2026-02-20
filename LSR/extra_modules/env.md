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
