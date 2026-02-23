
set -x

cd build
cmake ..
make -j 4
cd ..

set +x