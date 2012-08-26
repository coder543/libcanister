//just some helper functions to assist in standardized
//interpretation of the canister file
#include "fileinterpretation.h"
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