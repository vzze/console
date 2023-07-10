param(
    [ValidateSet("Release", "RelWithDebInfo", "Debug")]
    [String]$build = "Debug"
)

Invoke-Expression "cmake --build ./build --config $build"
