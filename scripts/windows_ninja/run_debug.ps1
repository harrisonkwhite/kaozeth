$ErrorActionPreference = "Stop"

$ProjectRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$BuildDir = Join-Path $ProjectRoot "build/debug"

cmake -S "$ProjectRoot" -B "$BuildDir" -G Ninja -DCMAKE_BUILD_TYPE=Debug
if ($LASTEXITCODE -ne 0) { exit 1 }

cmake --build "$BuildDir"
if ($LASTEXITCODE -ne 0) { exit 1 }

Push-Location $BuildDir
try {
    & ".\kaozeth.exe"
}
finally {
    Pop-Location
}
