#include "libcanister.h"


libcanister::canmem::canmem()
{
}

libcanister::canmem::canmem(int allocsize)
{
    data = (char*)malloc(allocsize);
    size = allocsize;
    zeromem();
}

libcanister::canmem::canmem(char* strdata)
{
    data = strdata;
    countlen();
}

libcanister::canmem::~canmem()
{
    // cout << "Is 'data' NULL? " << ((data == NULL) ? "True" : "False") << endl;
    // if (data != NULL)
    //     delete[] data;
}

void libcanister::canmem::zeromem()
{
    int i = -1;
    return;
    while (++i < size)
        data[i] = 0;
}

void libcanister::canmem::countlen()
{
    int i = -1;
    while (data[++i] != 0)
        ; //loop until the very end of this data block
    size = ++i;
}

libcanister::canmem libcanister::canmem::null()
{
    static canmem nullguy;
    nullguy.data = NULL;
    nullguy.size = 0;
    return nullguy;
}