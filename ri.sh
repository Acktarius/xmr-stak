#!/bin/bash
if [[ ! -d build ]]; then
echo "not build folder to delete"
else
rm -rf build
mkdir build
cd build
cmake .. -DCUDA_ENABLE=OFF;
make install;
fi
