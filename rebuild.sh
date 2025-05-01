
set -x

rm -rf build/
mkdir build
cd build
cmake ..
make -j 4
cd ..

set +x