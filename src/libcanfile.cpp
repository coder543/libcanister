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
        dout << "file " << path.data << " not in memory -- caching." << endl;
        //open the canister once again
        ifstream infile;
        infile.open(parent->info.path.data);
        int i = 0;
        infile.seekg(14, ios::beg); //seek to file section
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
        if (id & 0x80000000)
        {
            cachestate = -1;
            data = *(new canmem((char*)"This is a fragment. Not being read to memory."));
            return;
        }
        else if (id == cfid)
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
            cerr << "Error: canister is corrupt. CFID Code: " << id << ":" << cfid << endl;
            cachestate = -1;
            data = *(new canmem((char*)"Sorry -- canister is corrupted somehow."));
        }
    }
    else
        dout << "file " << path.data << " already in memory -- not caching." << endl;
}

void libcanister::canfile::cachedump()
{
    if (cachestate == 2 && !parent->readonly) //time to dump the cache
    {
        canmem cmpdata;
        if (!isfrag)
            cmpdata = bzipWrapper::compress(data);
        else
            cmpdata = data;
        dsize = cmpdata.size;
        fstream infile;
        infile.open(parent->info.path.data, ios::in | ios::out | ios::binary);
        int i = 0;
        infile.seekg(14, ios::beg); //seek to file section
        //loop through the files
        while (i < parent->info.numfiles)
        {
            dout << "in loop" << endl;
            dout << "sloc: " << infile.tellg() << endl;
            //int id = readint32(infile);
            //readint32 (needed because fstream != ifstream)
            unsigned char temp;
            unsigned int id = 0;
            if (!infile.good())
            {
                infile >> temp;
                id = temp * 256 * 256 * 256;
                dout << (int)temp << " ";
                
                infile >> temp;
                id += temp * 256 * 256;
                dout << (int)temp << " ";
                
                infile >> temp;
                id += temp * 256;
                dout << (int)temp << " ";
                
                infile >> temp;
                id += temp;
                dout << (int)temp << " ";

                if (parent->files[i].cfid == cfid && cfid == id)
                {
                    dout << "rewriting existing file." << endl;
                    infile.write(cmpdata.data, cmpdata.size);
                    break;
                }
                else if (parent->files[i].cfid != id)
                {
                    infile.seekg(-4, ios::cur);
                    dout << "writing new file" << endl;

                    unsigned char* id = (unsigned char*)(void*)&cfid;
                    infile << id[3];
                    infile << id[2];
                    infile << id[1];
                    infile << id[0];

                    infile.write(cmpdata.data, cmpdata.size);
                    break;
                }
            }
            else
            {
                dout << "writing new file" << endl;

                unsigned char* id = (unsigned char*)(void*)&cfid;
                infile << id[3];
                infile << id[2];
                infile << id[1];
                infile << id[0];

                infile.write(cmpdata.data, cmpdata.size);
                break;
            }
            //seek to the beginning of the next file
            infile.seekg(parent->files[i++].dsize, ios::cur);
            dout << "eloc: " << infile.tellg() << endl;
        }
        infile.close();
    }
}


void libcanister::canfile::cachedumpfinal(fstream& infile)
{
    //#warning "Need to implement cachedump."

    if (cachestate == 2 && !parent->readonly) //time to dump the cache
    {
        canmem cmpdata;
        if (!isfrag)
            cmpdata = bzipWrapper::compress(data);
        else
            cmpdata = data;
        dsize = cmpdata.size;
        //int id = readint32(infile);
        //readint32 (needed because fstream != ifstream)
        if (!infile.good())
        {
            unsigned char temp;
            unsigned int id = 0;
            
            infile >> temp;
            id = temp * 256 * 256 * 256;
            dout << (int)temp << " ";
            
            infile >> temp;
            id += temp * 256 * 256;
            dout << (int)temp << " ";
            
            infile >> temp;
            id += temp * 256;
            dout << (int)temp << " ";
            
            infile >> temp;
            id += temp;
            dout << (int)temp << " ";
            if (cfid == id)
                infile.write(cmpdata.data, cmpdata.size);
            else if (cfid != id)
            {
                infile.seekg(-4, ios::cur);
                dout << "writing new file" << endl;

                unsigned char* id = (unsigned char*)(void*)&cfid;
                infile << id[3];
                infile << id[2];
                infile << id[1];
                infile << id[0];

                infile.write(cmpdata.data, cmpdata.size);
            }
        }
        else
        {
            dout << "writing new file" << endl;

            unsigned char* id = (unsigned char*)(void*)&cfid;
            infile << id[3];
            infile << id[2];
            infile << id[1];
            infile << id[0];
            infile.write(cmpdata.data, cmpdata.size);
        }
    }
    else
        infile.seekg(dsize+4, ios::cur);
}