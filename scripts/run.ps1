# This needs to be run from project root.

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"
$projectRoot = $PWD

mkdir build -Force | Out-Null

Push-Location build
try {
    cmake .. "-DCMAKE_BUILD_TYPE=$Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    & cmake --build .
    $buildExitCode = $LASTEXITCODE

    $src = Join-Path (Get-Location) "compile_commands.json"
    $dest = Join-Path $projectRoot "compile_commands.json"

    if (Test-Path $src) {
        Copy-Item $src $dest -Force
    }

    $exe = Join-Path (Get-Location) "kaozeth.exe"
} finally {
    Pop-Location
}

if ($buildExitCode -ne 0) {
    Write-Host "Build failed with exit code $buildExitCode."
    exit $buildExitCode
}

if (Test-Path $exe) {
    Push-Location $projectRoot
    & $exe
    Pop-Location
}
