$ProjectRoot = Resolve-Path "$PSScriptRoot\.."
$ErrorActionPreference = "Stop"

cmake -S "$ProjectRoot" -B "$ProjectRoot/build/release" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$ProjectRoot/build/release"

& "$ProjectRoot/build/release/kaozeth.exe"
