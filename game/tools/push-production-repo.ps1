param(
    [string]$Remote = "https://github.com/BelialRunnerX/production-build.git",
    [string]$Branch = "main"
)
$ErrorActionPreference = "Stop"
Set-Location (Resolve-Path (Join-Path $PSScriptRoot ".."))
if (!(Test-Path ".git")) { git init -b $Branch }
git add -A
if (-not (git diff --cached --quiet)) {
    git -c user.name="Elysium Production" -c user.email="elysium-production@users.noreply.github.com" commit -m "Import reconciled Elysium production build"
}
$existing = git remote 2>$null | Select-String -SimpleMatch "origin"
if ($existing) { git remote set-url origin $Remote } else { git remote add origin $Remote }
git push -u origin $Branch
