$ProjectRoot = Resolve-Path "$PSScriptRoot\.."
$ErrorActionPreference = "Stop"

cmake -S "$ProjectRoot" -B "$ProjectRoot/build/debug" -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build "$ProjectRoot/build/debug"

& "$ProjectRoot/build/debug/kaozeth.exe"
