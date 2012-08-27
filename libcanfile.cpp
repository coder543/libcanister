#include "libcanister.h"
#include "fileinterpretation.h"
#include "bzipWrapper.h"    

//pull file from disk into memory if needed
void libcanister::canfile::cache()
{
    if (isfrag) //we aren't caching unallocated fragments -- sorry
    {
        cachestate = -1;
        data = *(new canmem((char*)"Sorry -- can't read fragments into memory."));
    }
    else if (!cachestate) //if it isn't cached, we want it
    {
        //open the canister once again
        ifstream infile;
        infile.open(parent->info.path.data);
        int i = 0;
        char tmp = 0;
        //seek to the end of the header
        while (tmp != 0x03)
            infile >> tmp;
        //loop through the files
        while (i < parent->info.numfiles)
        {
            //if we're at the correct file
            if (parent->files[i].cfid == cfid)
                //then stop
                break;
            //seek to the beginning of the next file
            infile.seekg(parent->files[i++].dsize+4, ios::cur);
            dout << "loc: " << infile.tellg() << endl;
        }
        //read the id as it is written in the canister
        int id = readint32(infile);
        //does the id match correctly? and make sure this is no fragment
        if (id == cfid || id == (cfid + 0xFF000000))
        {
            //now then, we need to create a new data member to hold the
            //compressed file contents
            data = *(new canmem(dsize));
            //read them into memory
            infile.read(data.data, dsize);
            //inflate the data
            data = bzipWrapper::inflate(data);
            //mark the cache state to be clean
            cachestate = 1;
            //let the canister know we've got more cached
            parent->cachecnt++;
            // if need be, then clean the cache
            if (parent->cachecnt > parent->cachemax)
                parent->cacheclean(cfid);
        }
        else
        { //otherwise we have a problem
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