if [ -d "build" ]; then
    rm -r "build"
fi

if [ -z $1 ]; then
    echo "Specify a build type: Release, RelWithDebInfo, Debug"
    exit 1
fi

build_type="$1"

build_type=$(tr '[:upper:]' '[:lower:]' <<< $build_type)

declare -A build_opts=(
    ["release"]=Release
    ["relwithdebinfo"]=RelWithDebInfo
    ["debug"]=Debug
)

if [ -z ${build_opts["$build_type"]} ]; then
    echo "Supported build types: Release, RelWithDebInfo, Debug"
    exit 1
fi

cmake -DCMAKE_BUILD_TYPE=${build_opts["$build_type"]} -S . -B build

exit 0
