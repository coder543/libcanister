//just some helper functions to assist in standardized
//interpretation of the canister file
#include "fileinterpretation.h"

//reads a 0x02-limited string of maximum length 512 from infile
libcanister::canmem readstr(ifstream& infile)
{
    int i = 1;
    char* str = (char*)malloc(512);
    infile >> str[0];
    while (i < 512 && str[i-1] != 2)
        infile >> str[i++];
    str[i-1] = 0;
    libcanister::canmem intername;
    intername.data = (char*)malloc(i);
    memcpy(intername.data, str, i);
    intername.size = i;
    free(str);
    return intername;
}

//reads a 32-bit integer from the infile stream
unsigned int readint32(ifstream& infile)
{
    unsigned char temp;
    unsigned int result = 0;
    
    infile >> temp;
    result = temp * 256 * 256 * 256;
    dout << (int)temp << " ";
    
    infile >> temp;
    result += temp * 256 * 256;
    dout << (int)temp << " ";
    
    infile >> temp;
    result += temp * 256;
    dout << (int)temp << " ";
    
    infile >> temp;
    result += temp;
    dout << (int)temp << " ";
    
    return result;
}

//similarly, reads a 64-bit integer from the infile stream
int64 readint64(ifstream& infile)
{
    unsigned char temp;
    int64 result = 0;
    
    infile >> temp;
    result = temp * (int64)256 * 256 * 256 * 256 * 256 * 256 * 256;
    dout << (int64)temp << " ";
    
    
    infile >> temp;
    result += temp * (int64)256 * 256 * 256 * 256 * 256 * 256;
    dout << (int64)temp << " ";
    
    
    infile >> temp;
    result += temp * (int64)256 * 256 * 256 * 256 * 256;
    dout << (int64)temp << " ";
    
    
    infile >> temp;
    result += temp * (int64)256 * 256 * 256 * 256;
    dout << (int64)temp << " ";
    
    
    infile >> temp;
    result += temp * 256 * 256 * 256;
    dout << (int64)temp << " ";
    
    infile >> temp;
    result += temp * 256 * 256;
    dout << (int64)temp << " ";
    
    infile >> temp;
    result += temp * 256;
    dout << (int64)temp << " ";
    
    infile >> temp;
    result += temp;
    dout << (int64)temp << " ";
    
    return result;
}