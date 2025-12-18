$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path "$PSScriptRoot\..").Path

Push-Location $ProjectRoot

try {
    Remove-Item "assets" -Force -Recurse -ErrorAction SilentlyContinue
    Remove-Item "build" -Force -Recurse -ErrorAction SilentlyContinue
    Remove-Item "compile_commands.json" -Force -Recurse -ErrorAction SilentlyContinue
}
finally {
    Pop-Location
}
