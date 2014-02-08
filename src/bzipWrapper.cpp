#include "bzipWrapper.h"
#include <bzlib.h>

//obviously these are just prototypes

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
    return rawdata;
    unsigned int compdatamax = rawdata.size*1.01+600;
    if (extraneeded)
    	compdatamax *= 2;
    libcanister::canmem compdata = *(new libcanister::canmem(compdatamax));
    compdata.zeromem();
    int result = BZ2_bzBuffToBuffCompress(compdata.data, &compdatamax, rawdata.data, rawdata.size, 7, 4, 30);
    switch (result)
    {
    	case BZ_CONFIG_ERROR:
    		cerr << "Major problem: the bzip library isn't working right." << endl;
    		exit(3);
		break;
    	case BZ_PARAM_ERROR:
    		cerr << "This isn't possible. BZ_PARAM_ERROR just happened." << endl;
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
    		compdata.size = compdatamax;
    		return compdata;
    	break;
    }
}

// typedef struct {
//   char *next_in;
//   unsigned int avail_in;
//   unsigned int total_in_lo32;
//   unsigned int total_in_hi32;

//   char *next_out;
//   unsigned int avail_out;
//   unsigned int total_out_lo32;
//   unsigned int total_out_hi32;

//   void *state;

//   void *(*bzalloc)(void *,int,int);
//   void (*bzfree)(void *,void *);
//   void *opaque;
// } bz_stream;

libcanister::canmem bzipWrapper::inflate(libcanister::canmem &compdata)
{
    #warning "Need to implement inflation."
    return compdata;    
}