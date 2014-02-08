#!/bin/bash
#Build and Run (bar)

cd src

clear
echo "      Building libcanister"

g++ -c libcanmem.cpp libcanister.cpp libcanfile.cpp fileinterpretation.cpp bzipWrapper.cpp &&

echo "      Building canisterdemo" &&

g++ canisterdemo.cpp fileinterpretation.cpp libcanister.o libcanmem.o bzipWrapper.o libcanfile.o -lbz2 -o ../bin/canidemo.bin &&

echo "      Building comptester" &&

g++ comptester.cpp libcanmem.o bzipWrapper.o -lbz2 -o ../bin/comptester.bin &&

rm *.o &&

echo "      Running  canisterdemo" &&
echo "" &&
echo "=====================================" &&
cd ../bin/
./canidemo.bin ../resources/candemo
