#include "bzipWrapper.h"
#include <bzlib.h>

/*

int BZ2_bzBuffToBuffCompress( char*         dest,
                              unsigned int* destLen,
                              char*         source,
                              unsigned int  sourceLen,
                              int           blockSize100k,
                              int           verbosity,
                              int           workFactor );

*/

libcanister::canmem bzipWrapper::compress(libcanister::canmem &rawdata, int extraneeded)
{
    unsigned int compdatamax = rawdata.size*1.01+600;
    if (extraneeded)
    	compdatamax *= 2;
    libcanister::canmem compdata = *(new libcanister::canmem(compdatamax));
    compdata.zeromem();
    int result = BZ2_bzBuffToBuffCompress(compdata.data, &compdatamax, rawdata.data, rawdata.size, 7, 0, 30);
    switch (result)
    {
    	case BZ_CONFIG_ERROR:
    		cerr << "Major problem: the bzip library isn't working right." << endl;
    		exit(3);
		break;
    	case BZ_PARAM_ERROR:
    		cerr << "This shouldn't be possible. BZ_PARAM_ERROR just happened." << endl;
    		exit(4);
    	break;
    	case BZ_MEM_ERROR:
    		cerr << "We've run out of memory, sorry about this." << endl;
    		exit(5);
    	break;
    	case BZ_OUTBUFF_FULL:
    		cerr << "Seems the output buffer overflowed.. fixing." << endl;
    		return compress(rawdata, 1);
    	break;
    	case BZ_OK:
    		//compdatamax will have been altered by the BZ2 function
            //let's allocate a new canmem object to contain the data less wastefully
            //the other will be deallocated when this function returns
            libcanister::canmem finaldata = *(new libcanister::canmem(compdatamax + 1 + sizeof(unsigned int)));
            finaldata.data[0] = 1; //version of this ad-hoc method of storing the size of the uncompressed data
            unsigned int* uncompsize = (unsigned int*)(finaldata.data + 1);
            *uncompsize = rawdata.size; //record the size of the uncompressed data
            memcpy(finaldata.data + 1 + sizeof(unsigned int), compdata.data, compdatamax);
    		return finaldata;
    	break;
    }
    return libcanister::canmem::null();
}

libcanister::canmem bzipWrapper::inflate(libcanister::canmem &compdata, int extra)
{
    if (compdata.data[0] == 1)
    { //we support this version! excellent.
        unsigned int* uncompsize_ptr = (unsigned int*)(compdata.data + 1);
        unsigned int uncompsize = *uncompsize_ptr;
        uncompsize *= extra;
        libcanister::canmem finaldata = *(new libcanister::canmem(uncompsize));
        int result = BZ2_bzBuffToBuffDecompress(finaldata.data, &uncompsize, compdata.data + 1 + sizeof(unsigned int), compdata.size - 1 - sizeof(unsigned int), 0, 0);
        switch (result)
        {
            case BZ_CONFIG_ERROR:
                cerr << "Major problem: the bzip library isn't working right." << endl;
                exit(6);
            break;
            case BZ_PARAM_ERROR:
                cerr << "This shouldn't be possible. BZ_PARAM_ERROR just happened." << endl;
                exit(7);
            break;
            case BZ_MEM_ERROR:
                cerr << "We've run out of memory, sorry about this." << endl;
                exit(8);
            break;
            case BZ_OUTBUFF_FULL:
                cerr << "Seems the output buffer overflowed.. fixing." << endl;
                return inflate(compdata, extra + 1);
            break;
            case BZ_UNEXPECTED_EOF:
            case BZ_DATA_ERROR_MAGIC:
            case BZ_DATA_ERROR:
                cerr << "This file is corrupt, returning null." << endl;
                return libcanister::canmem::null();
            break;
            case BZ_OK:
                return finaldata;
            break;
        }
    }
    return libcanister::canmem::null();
}