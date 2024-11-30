#!/bin/bash
if [[ -d build ]]; then
rm -rf build
fi
mkdir build
cd build
cmake .. -DCUDA_ENABLE=OFF;
make install;

