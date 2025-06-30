# Git大文件处理说明

## 问题说明
Git本身不直接支持按文件大小忽略文件。`.gitignore`文件主要基于文件名和路径模式进行过滤。

## 解决方案

### 方案1：使用Git LFS（推荐）
Git LFS (Large File Storage) 是处理大文件的标准方案。

#### 安装Git LFS：
```bash
# Windows (使用Git for Windows自带)
git lfs install

# Ubuntu/Debian
sudo apt-get install git-lfs

# macOS
brew install git-lfs
```

#### 配置Git LFS：
在项目根目录创建 `.gitattributes` 文件：

```bash
# .gitattributes 文件内容
*.pdf filter=lfs diff=lfs merge=lfs -text
*.doc filter=lfs diff=lfs merge=lfs -text
*.docx filter=lfs diff=lfs merge=lfs -text
*.zip filter=lfs diff=lfs merge=lfs -text
*.rar filter=lfs diff=lfs merge=lfs -text
*.7z filter=lfs diff=lfs merge=lfs -text
*.iso filter=lfs diff=lfs merge=lfs -text
*.img filter=lfs diff=lfs merge=lfs -text
*.bin filter=lfs diff=lfs merge=lfs -text
*.hex filter=lfs diff=lfs merge=lfs -text
```

### 方案2：使用Git钩子脚本
创建一个pre-commit钩子来检查文件大小：

#### 创建 `.git/hooks/pre-commit` 文件：
```bash
#!/bin/bash
# Pre-commit hook to check file sizes

# 设置文件大小限制 (50MB = 52428800 bytes)
MAX_SIZE=52428800

# 检查暂存区的文件
for file in $(git diff --cached --name-only); do
    if [ -f "$file" ]; then
        file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null)
        if [ "$file_size" -gt $MAX_SIZE ]; then
            echo "错误: 文件 '$file' 大小为 $(($file_size / 1024 / 1024))MB，超过了50MB的限制"
            echo "请使用 Git LFS 或将文件添加到 .gitignore"
            exit 1
        fi
    fi
done
```

### 方案3：查找并处理现有大文件
使用以下命令查找项目中的大文件：

#### Windows PowerShell：
```powershell
# 查找大于50MB的文件
Get-ChildItem -Recurse | Where-Object {$_.Length -gt 50MB} | Select-Object Name, Length, FullName
```

#### Linux/macOS：
```bash
# 查找大于50MB的文件
find . -type f -size +50M -exec ls -lh {} \;
```

### 方案4：清理已提交的大文件
如果已经提交了大文件，需要从Git历史中删除：

```bash
# 使用git filter-branch清理大文件（危险操作，请先备份）
git filter-branch --tree-filter 'rm -f path/to/large/file' HEAD

# 或使用BFG Repo-Cleaner（更安全）
java -jar bfg.jar --strip-blobs-bigger-than 50M my-repo.git
```

## 当前项目配置

已在 `.gitignore` 中添加了常见的大文件类型：
- 编译输出文件（*.axf, *.hex, *.bin等）
- 压缩文件（*.zip, *.rar, *.7z等）
- 多媒体文件（*.mp4, *.mp3等）
- 数据库文件（*.db, *.sqlite等）

## 建议操作步骤

1. **检查现有大文件**：
   ```bash
   find . -type f -size +50M
   ```

2. **决定处理方式**：
   - 删除不需要的大文件
   - 将需要的大文件用Git LFS管理
   - 将临时大文件添加到.gitignore

3. **初始化Git LFS**（如果需要）：
   ```bash
   git lfs install
   git lfs track "*.pdf"
   git add .gitattributes
   ```

4. **提交更改**：
   ```bash
   git add .gitignore
   git commit -m "添加.gitignore文件，配置大文件过滤"
   ```

## 注意事项
- Git LFS需要远程仓库支持（GitHub、GitLab等都支持）
- 大文件一旦提交到Git，即使删除也会占用仓库大小
- 建议在项目开始时就配置好大文件处理策略
