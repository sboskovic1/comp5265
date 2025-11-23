#!/bin/bash
clear
gcc *.c ./yacsim.o -lm -o main
./main --trace 0 --workload sharedArray --cpuDelay 20 --numProcs 32