$ErrorActionPreference = "Stop"

& "$PSScriptRoot\build_release.ps1"
if ($LASTEXITCODE -ne 0) { exit 1 }

$ProjectRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
& "$ProjectRoot\build\release\kaozeth.exe"
