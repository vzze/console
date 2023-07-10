$check_dir = Test-Path ./build -PathType Container

if($check_dir -eq $true) {
    Remove-Item build -Recurse
}

Invoke-Expression "cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -S . -B build -G `"MinGW Makefiles`""
