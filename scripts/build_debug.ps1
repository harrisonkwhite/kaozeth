$ErrorActionPreference = "Stop"
$ProjectRoot = Resolve-Path "$PSScriptRoot\.."
cmake -S $ProjectRoot -B "$ProjectRoot/build/debug" -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build "$ProjectRoot/build/debug"
