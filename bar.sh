#!/bin/bash
#Build and Run (bar)

clear
echo "      Building libcanister"

g++ -c libcanmem.cpp libcanister.cpp libcanfile.cpp fileinterpretation.cpp bzipWrapper.cpp &&

echo "      Building canisterdemo" &&

g++ canisterdemo.cpp fileinterpretation.cpp libcanister.o libcanmem.o bzipWrapper.o libcanfile.o -lbz2 -o canidemo.bin &&

rm *.o &&

echo "      Running  canisterdemo" &&
echo "" &&
echo "=====================================" &&

sleep 1; rm ./canidemo.bin & ./canidemo.bin candemo