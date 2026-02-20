# rumina:path

```lamina
include "rumina:path"
```

路径处理工具。

## 方法与属性

- `path.join(paths: List<String>) -> String`
  拼接路径片段。
- `path.basename(p: String) -> String`
  取最后路径段。
- `path.dirname(p: String) -> String`
  取父目录路径。
- `path.extname(p: String) -> String`
  取扩展名（含点）。
- `path.isAbsolute(p: String) -> Bool`
  判断是否绝对路径。
- `path.normalize(p: String) -> String`
  规整 `.` / `..`。
- `path.resolve(paths: List<String>) -> String`
  从 `cwd` 解析为绝对路径。
- `path.relative(from: String, to: String) -> String`
  计算相对路径。
- `path.parse(p: String) -> Dict`
  返回 `{root,dir,base,ext,name}`。
- `path.format(parts: Dict) -> String`
  由对象构建路径；`ext` 无点时自动补点。
- `path.sep -> String`
  平台路径分隔符。
- `path.delimiter -> String`
  PATH 分隔符。
