#include "libcanister.h"

class bzipWrapper
{
    public:
    static libcanister::canmem compress(libcanister::canmem &rawdata);
    static libcanister::canmem inflate(libcanister::canmem &compdata);
};