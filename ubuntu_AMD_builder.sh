#!/bin/bash
if [[ ! -d build ]]; then
mkdir build
else
rm -rf build
fi
cd build
cmake .. -DCUDA_ENABLE=OFF;
make install;

