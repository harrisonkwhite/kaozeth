$ErrorActionPreference = "Stop"

& "$PSScriptRoot\build_debug.ps1"
if ($LASTEXITCODE -ne 0) { exit 1 }

$ExePath = Resolve-Path "$PSScriptRoot\..\..\build\debug\zeta_framework\zf_tests\zf_tests.exe"
$ExeDir  = Split-Path $ExePath -Parent

Push-Location $ExeDir
& $ExePath
Pop-Location
