#!/bin/bash
rm -rf *
source /opt/toolchain/environment-setup-aarch64-xilinx-linux
cmake ..
make clean; make -j16
