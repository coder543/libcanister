#!/bin/bash
clear
echo "      Building libcanister"

g++ -c libcanmem.cpp libcanister.cpp libcanfile.cpp fileinterpretation.cpp bzipWrapper.cpp &&

echo "      Building canisterdemo" &&

g++ canisterdemo.cpp fileinterpretation.cpp libcanister.o libcanmem.o bzipWrapper.o libcanfile.o -o canidemo.bin &&

echo "      Running  canisterdemo" &&
echo "" &&
echo "=====================================" &&

./canidemo.bin