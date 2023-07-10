if [ -d "build" ]; then
    rm -r "build"
fi

export CXX=clang++
export CC=clang

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -S . -B build
