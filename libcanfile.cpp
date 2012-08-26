#include "libcanister.h"
#include "fileinterpretation.h"
#include "bzipWrapper.h"	

void libcanister::canfile::cache()
{
	if (isfrag)
	{
		cachestate = -1;
		data = *(new canmem((char*)"Sorry -- can't read fragments into memory."));
	}
	if (!cachestate) //if it isn't cached
	{
		ifstream infile;
		infile.open(parent->info.path.data);
		int i = 0;
		char tmp = 0;
		while (tmp != 0x03)
			infile >> tmp;
		while (i < parent->info.numfiles)
		{
			if (parent->files[i].cfid == cfid)
				break;
			infile.seekg(parent->files[i++].dsize+4, ios::cur);
			dout << "loc: " << infile.tellg() << endl;
		}
		int id = readint32(infile);
		if (id == cfid || id == (cfid + 0xFF000000))
		{
			data = *(new canmem(dsize));
			infile.read(data.data, dsize);
			data = bzipWrapper::inflate(data);
			cachestate = 1;
			parent->cachecnt++;
			if (parent->cachecnt > parent->cachemax)
				parent->cacheclean(cfid);
		}
		else
		{
			cout << "Error: canister is corrupt." << endl;
			cachestate = -1;
			data = *(new canmem((char*)"Sorry -- canister is corrupted somehow."));
		}
	}
}

void libcanister::canfile::cachedump()
{
	//to be implemented
	cout << "UNIMPLEMENTED CACHEDUMP" << endl;
	#warning "Need to implement cachedump."
	data = bzipWrapper::compress(data);
}