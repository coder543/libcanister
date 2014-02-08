#include "bzipWrapper.h"
#include "libcanister.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace libcanister;

#define one_megabyte 1024*1024

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
	int length = input.tellg();
	length = length > one_megabyte ? one_megabyte : length;
	input.read(buffer, length);
	input.close();
	
	canmem rawdata;
	rawdata.data = buffer;
	rawdata.size = length;
	ofstream compressed;
	compressed.open("compressed");
	canmem compdata = bzipWrapper::compress(rawdata);
	compressed.write(compdata.data, compdata.size);
	compressed.close();
	
	ofstream inflated;
	inflated.open("inflated");
	canmem inflateddata = bzipWrapper::inflate(compdata);
	inflated.write(inflateddata.data, inflateddata.size);
	inflated.close();
}