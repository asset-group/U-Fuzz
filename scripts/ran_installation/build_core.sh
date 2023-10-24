#!/usr/bin/env bash

export WDISSECTOR_PATH=$(pwd)/wdissector/


# -------------------- Build Open5gs-Core -----------------------------
echo "Downloading open5gs-core with the fuzzing additions"
git clone https://gitlab.com/asset-sutd/software/open5gs-core --depth=1
cd open5gs-core
# Build and install requirements
# libzmq
git clone https://github.com/zeromq/libzmq.git --depth=1
cd libzmq
./autogen.sh
./configure
make -j
sudo make install
sudo ldconfig
cd ../

# libczmq
git clone https://github.com/zeromq/czmq.git --depth=1
cd czmq
./autogen.sh
./configure
make -j
sudo make install
sudo ldconfig
cd ../

# libjson-c
git clone https://github.com/json-c/json-c.git --depth=1
cd json-c
mkdir build
cd build
cmake ../
make -j
sudo make install
cd ../../

# build open5gs-core
./build.sh
# install webui dependencies
cd webui
npm install

cd ../../

echo "Done! run cd open5gs-core && ./run.sh"
