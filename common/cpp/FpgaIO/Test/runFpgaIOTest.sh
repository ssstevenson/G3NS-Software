#!/bin/sh
#  program   ProcessName Iter Mask value
./fpgaIoTest Process_1   500 0x80000000 &
./fpgaIoTest Process_2   500 0x40000000 &
./fpgaIoTest Process_3   500 0x20000000 &
./fpgaIoTest Process_4   500 0x08000000 &
./fpgaIoTest Process_5   500 0x04000000 &
./fpgaIoTest Process_5   500 0x02000000 &

