#!/bin/bash
#Build and Run (bar)

#CC is the compiler
#CC=g++
CC=clang++
#SA is the static analyzer
#(comment out next line if there is no static analyzer)
SA="scan-build -k"

cd src
clear
echo "      Building libcanister"

$SA $CC $1 -Wall -c libcanmem.cpp libcanister.cpp libcanfile.cpp fileinterpretation.cpp bzipWrapper.cpp &&

echo "      Building canisterdemo" &&

$SA $CC $1 -Wall canisterdemo.cpp fileinterpretation.cpp libcanister.o libcanmem.o bzipWrapper.o libcanfile.o -lbz2 -o ../bin/canidemo.bin &&

rm *.o &&

echo "      Running  canisterdemo" &&
echo "" &&
echo "=====================================" &&
cd ../bin/

if [ "$1" == "-g" ]
then
	gdb --args ./canidemo.bin ../resources/candemo
else
	./canidemo.bin ../resources/candemo
fi
