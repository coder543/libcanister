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
    while (++i < size)
        data[i] = 0;
}

void libcanister::canmem::fragmem()
{
    if (size <= 0)
        return;
    int i = -1;
    while (++i < size)
        data[i] = 0xFF;
    data[i-1] = 0x02;
}

void libcanister::canmem::countlen()
{
    int i = -1;
    while (data[++i] != 0); //loop until the very end of this data block
    size = ++i;
    if (size > 0)
        size++; //grab the null-char at the end of the string
}

void libcanister::canmem::trim()
{
    //handle some funkyness. (specifically, the last two bytes of the file, 0x0203, would get duplicated once.)
    if (size < 1)
        return;
    while ((data[size-1] == 0x00 || data[size-1] == 0x02 || data[size-1] == 0x03) && size > 1)
        size--;
}

libcanister::canmem libcanister::canmem::null()
{
    static canmem nullguy;
    nullguy.data = NULL;
    nullguy.size = 0;
    return nullguy;
}