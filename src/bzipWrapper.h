#include "libcanister.h"

class bzipWrapper
{	
public:
    static libcanister::canmem compress(libcanister::canmem &rawdata, int extra = 0);
    static libcanister::canmem inflate(libcanister::canmem &compdata);
};