
set -x

mkdir -p build
cd build
cmake ..
make -j 4
cd ..

set +x