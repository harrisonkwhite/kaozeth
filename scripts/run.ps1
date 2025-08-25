# This needs to be run from project root.

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"
$projectRoot = $PWD

mkdir build -Force | Out-Null

try {
    Push-Location build

    cmake .. "-DCMAKE_BUILD_TYPE=$Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build .

    $src = Join-Path (Get-Location) "compile_commands.json"
    $dest = Join-Path $projectRoot "compile_commands.json"

    if (Test-Path $src) {
        Copy-Item $src $dest -Force
    }

    $exe = Join-Path (Get-Location) "terraria_clone.exe"

    if (Test-Path $exe) {
        Push-Location $projectRoot
        & $exe
        Pop-Location
    }
} finally {
    Pop-Location
}
