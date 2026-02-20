# rumina:path

```lamina
include "rumina:path"
```

导入后可使用 `path` 对象：`path.xxx(...)`。

- `path.join(paths: List<String>) -> String`
- `path.basename(p: String) -> String`
- `path.dirname(p: String) -> String`
- `path.extname(p: String) -> String`
- `path.isAbsolute(p: String) -> Bool`
- `path.normalize(p: String) -> String`

## 用法示例

```lamina
include "rumina:path";

var p = path.join(["/var", "log/", "app.log"]);
print(p);                      // /var/log/app.log
print(path.basename(p));       // app.log
print(path.dirname(p));        // /var/log
print(path.extname(p));        // .log
print(path.isAbsolute(p));     // true
print(path.normalize("/a/b/../c/./x.txt"));
```

## 注意事项

- `join` 参数必须是 `List<String>`。
- `normalize` 只做路径语义归一化，不会检查路径是否真实存在。
