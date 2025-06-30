# 检查项目中大文件的PowerShell脚本
# 作者: Mika
# 使用方法: 在PowerShell中运行 .\check_large_files.ps1

param(
    [int]$SizeLimit = 50  # 默认50MB
)

Write-Host "正在检查大于 $SizeLimit MB 的文件..." -ForegroundColor Green
Write-Host "=" * 60

# 转换为字节
$SizeLimitBytes = $SizeLimit * 1024 * 1024

# 查找大文件
$LargeFiles = Get-ChildItem -Recurse -File | Where-Object {
    $_.Length -gt $SizeLimitBytes -and
    $_.FullName -notmatch "\.git\\" -and
    $_.FullName -notmatch "Objects\\" -and
    $_.FullName -notmatch "Listings\\"
}

if ($LargeFiles) {
    Write-Host "发现以下大文件:" -ForegroundColor Yellow
    Write-Host ""
    
    $LargeFiles | ForEach-Object {
        $SizeMB = [math]::Round($_.Length / 1024 / 1024, 2)
        $RelativePath = $_.FullName.Replace((Get-Location).Path + "\", "")
        
        Write-Host "📁 $RelativePath" -ForegroundColor Cyan
        Write-Host "   大小: $SizeMB MB" -ForegroundColor White
        Write-Host "   修改时间: $($_.LastWriteTime)" -ForegroundColor Gray
        Write-Host ""
    }
    
    Write-Host "建议操作:" -ForegroundColor Yellow
    Write-Host "1. 删除不需要的大文件" -ForegroundColor White
    Write-Host "2. 将重要的大文件移到其他位置" -ForegroundColor White
    Write-Host "3. 使用 Git LFS 管理必要的大文件" -ForegroundColor White
    Write-Host "4. 将大文件类型添加到 .gitignore" -ForegroundColor White
    
} else {
    Write-Host "✅ 未发现大于 $SizeLimit MB 的文件" -ForegroundColor Green
}

Write-Host ""
Write-Host "=" * 60
Write-Host "检查完成！" -ForegroundColor Green

# 显示项目总大小
$TotalSize = (Get-ChildItem -Recurse -File | Measure-Object -Property Length -Sum).Sum
$TotalSizeMB = [math]::Round($TotalSize / 1024 / 1024, 2)
Write-Host "项目总大小: $TotalSizeMB MB" -ForegroundColor Cyan
