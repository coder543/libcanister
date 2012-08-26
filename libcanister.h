#ifndef LIBCANIH
#define LIBCANIH
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>

#define int64 unsigned long long
//#define DEBUG

#ifdef DEBUG
#define dout cout
#else
#define dout if (0) cerr
#endif

using namespace std;

namespace libcanister
{

    class canmem
    {
    public:
        char* data;
        int size;
        canmem();
        canmem(int allocsize);
        canmem(char* strdata);
        ~canmem();
        void zeromem();
        void countlen(); //counts length of zero-limited strings
        static canmem null();
        
    };
    
    class caninfo
    {
    public:
        canmem path; //physical path
        canmem internalname; //a name for the canister
        int numfiles;
    };
    
    class canister;
    
    class canfile
    {
    public:
        libcanister::canister* parent; //the canister that holds this file
        canmem path; //internal path (name)
        canmem data; //the file's contents
        int isfrag; //0 = not fragment, 1 = fragment (ignore)
        int cfid; //canfile id
        int64 dsize; //ondisk size
        int cachestate; //0 = not in memory, 1 = in memory, 2 = in memory and needs flush
                        //-1 = error, check the data for the message
        void cache(); //pull the file from disk and cache it in memory
        void cachedump(); //deletes the contents of this file from the memory cache after assuring the on disk copy is up to date
        void flush(); //updates the on disk copy, but retains the memory cache
    };

    class canister
    {
        canfile TOC;
    public:
        caninfo info;
        canfile* files;
        int cachemax;
        int cachecnt;
        bool readonly;
        //accepts a physical location
        canister (canmem fspath);
        canister (char* fspath);
        ~canister();
        
        //open the fspath
        //does it exist?
        // | --- yes --- opening it (return 1)
        // | --- yes --- file is corrupted, halting (return -1)
        // | --- no  --- making a new one (return 0)
        int open();
        
        //close the canister, flush all buffers, clean up
        int close();
        
        //deletes the file at path inside this canister
        int delFile(canmem path);
        
        //pulls the contents of the file from disk or memory and returns it as a file
        canfile getFile(canmem path);
        
        //creates a file if it does not exist, otherwise overwrites
        //returns whether operation succeeded
        bool writeFile(canmem path, canmem data);
        bool writeFile(canfile file);
        
        //get the 'table of contents', a file containing a newline delimited
        //list of the file paths in the container which have contents
        canfile getTOC();

        void cacheclean(int sCFID, bool dFlush = false); //safe CFID (the file we want to avoid uncaching)
    };

}

#endif