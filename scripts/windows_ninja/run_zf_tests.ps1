$ErrorActionPreference = "Stop"

& "$PSScriptRoot\build_debug.ps1"
if ($LASTEXITCODE -ne 0) { exit 1 }

$ProjectRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
& "$ProjectRoot\build\debug\zeta_framework\zf_tests\zf_tests.exe"
