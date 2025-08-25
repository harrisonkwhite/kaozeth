param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"
$projectRoot = $PWD

mkdir build -Force | Out-Null

try {
    Push-Location build

    cmake .. -A x64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build . --config $Config

    $src = Join-Path (Get-Location) "compile_commands.json"
    $dst = Join-Path $projectRoot "compile_commands.json"

    if (Test-Path $src) {
        Copy-Item $src $dst -Force
    }

    $exe = Join-Path (Get-Location) "$Config/terraria_clone.exe"

    if (Test-Path $exe) {
        Push-Location $projectRoot
        & $exe
        Pop-Location
    }
} finally {
    Pop-Location
}
