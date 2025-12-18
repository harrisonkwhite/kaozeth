$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path "$PSScriptRoot\..").Path
$BuildDir = Join-Path $ProjectRoot "build/release"

cmake -S "$ProjectRoot" -B "$BuildDir" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BuildDir"

Push-Location $BuildDir

try {
    & ".\kaozeth.exe"
}
finally {
    Pop-Location
}
