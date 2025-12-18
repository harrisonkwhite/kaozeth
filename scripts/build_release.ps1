$ErrorActionPreference = "Stop"
$ProjectRoot = Resolve-Path "$PSScriptRoot\.."
cmake -S $ProjectRoot -B "$ProjectRoot/build/release" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$ProjectRoot/build/release"
