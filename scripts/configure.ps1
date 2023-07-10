param(
    [ValidateSet("MinGW", "MSVC", "ANY")]
    [String]$compiler = "ANY",
    [ValidateSet("Release", "RelWithDebInfo", "Debug")]
    [String]$build = "Debug"
)

$check_dir = Test-Path ./build -PathType Container

if($check_dir -eq $true) {
    Remove-Item build -Recurse
}

switch($compiler) {
    "MinGW" {
        Invoke-Expression "cmake -DCMAKE_BUILD_TYPE=$build -S . -B build -G `"MinGW Makefiles`""
    }

    "MSVC" {
        Invoke-Expression "cmake -S . -B build -G `"Visual Studio 17 2022`""
    }

    "ANY" {
        Invoke-Expression "cmake -DCMAKE_BUILD_TYPE=$build -S . -B build"
    }
}
