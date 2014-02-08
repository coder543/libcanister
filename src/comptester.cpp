#include "bzipWrapper.h"
#include "libcanister.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace libcanister;

#define one_megabyte 1024*1024

int error(int when)
{
	switch (when)
	{
		case 1:
			cout << "Error opening input." << endl;
		break;
		case 2:
			cout << "Error opening compressed." << endl;
		break;
		case 3:
			cout << "Error opening inflated." << endl;
		break;
	}
	return when;
}

int main(int argc, char** argv)
{
	char buffer[one_megabyte];
	if (argc < 2)
	{
		cout << "Must supply filename." << endl;
		return -1;
	}
	ifstream input;
	input.open(argv[1]);
	if (!input.good())
		return error(1);
	input.seekg(0, ios::end);
	int length = input.tellg();
	input.seekg(0, ios::beg);
	length = length > one_megabyte ? one_megabyte : length;
	input.read(buffer, length);
	input.close();
	
	canmem rawdata;
	rawdata.data = buffer;
	rawdata.size = length;
	ofstream compressed;
	compressed.open("compressed");
	if (!compressed.good())
		return error(2);
	canmem compdata = bzipWrapper::compress(rawdata);
	compressed.write(compdata.data, compdata.size);
	compressed.close();
	
	ofstream inflated;
	inflated.open("inflated");
	if (!inflated.good())
		return error(3);
	canmem inflateddata = bzipWrapper::inflate(compdata);
	inflated.write(inflateddata.data, inflateddata.size);
	inflated.close();
}