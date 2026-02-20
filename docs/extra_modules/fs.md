# rumina:fs

```lamina
include "rumina:fs"
```

同步文件系统 API。

## 文件读写

- `fs.readText(path: String, options?: Dict | String) -> String`
  读取文本。`options.encoding` 或字符串简写可为 `utf8`/`hex`/`base64`/`base64url`。
- `fs.readBytes(path: String, options?: Dict | String) -> Buffer`
  读取二进制字节。
- `fs.writeText(path: String, text: String, options?: Dict) -> Void`
  写文本。`options.flag` 支持 `w`（覆盖）/`a`（追加）。
- `fs.writeBytes(path: String, data: Buffer, options?: Dict) -> Void`
  写字节。`options.flag` 同上。
- `fs.append(path: String, data: String | Buffer) -> Void`
  追加文本或字节到文件末尾。

## 状态与检查

- `fs.exists(path: String) -> Bool`
  路径是否存在。
- `fs.isFile(path: String) -> Bool`
  是否为普通文件。
- `fs.isDir(path: String) -> Bool`
  是否为目录。
- `fs.stat(path: String) -> Dict`
  返回 `{ size, isFile, isDir, modifiedTime }`。

## 实体操作

- `fs.makeDir(path: String) -> Void`
  创建单层目录。
- `fs.makeDirAll(path: String) -> Void`
  递归创建目录。
- `fs.readDir(path: String, withTypes?: Bool) -> List<String | Dict>`
  读取目录项；`withTypes=true` 返回 `{name,isFile,isDir,isSymlink}`。
- `fs.remove(path: String) -> Void`
  删除文件或空目录。
- `fs.removeAll(path: String) -> Void`
  递归删除目录或删除文件。
- `fs.rename(oldPath: String, newPath: String) -> Void`
  重命名或移动。
- `fs.copy(srcPath: String, destPath: String) -> Void`
  复制文件。
- `fs.realpath(path: String) -> String`
  返回规范绝对路径（解析符号链接）。
- `fs.readLink(path: String) -> String`
  读取符号链接目标。
- `fs.link(existingPath: String, newPath: String) -> Void`
  创建硬链接。
- `fs.symlink(target: String, path: String) -> Void`
  创建符号链接。
- `fs.chmod(path: String, mode: Int) -> Void`
  修改权限（仅 Unix 支持）。

## 注意事项

- 当前为同步阻塞实现。
- `remove` 不会删除非空目录，请用 `removeAll`。
