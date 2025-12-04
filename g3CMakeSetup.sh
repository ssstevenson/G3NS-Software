#!/bin/bash
rm -rf build
mkdir build
cd build
source /opt/toolchain/environment-setup-aarch64-xilinx-linux
cmake ..
make clean; make -j16
