#include "libcanister.h"
#include "fileinterpretation.h"
#include "bzipWrapper.h"
using namespace std;


libcanister::canister::canister (char* fspath)
{
    info.path = *(new canmem(fspath));
    cachemax = 25;
    cachecnt = 0;
    readonly = false;
}

libcanister::canister::canister (canmem fspath)
{
    info.path = fspath;
    cachemax = 25;
    cachecnt = 0;
    readonly = false;
}

libcanister::canfile libcanister::canister::getTOC()
{
    return TOC;
}

libcanister::canfile libcanister::canister::getFile(canmem path)
{
    int i = 0;
    while (i < info.numfiles)
    {
        if(!strcmp(files[i].path.data, path.data))
        {
            files[i].cache();
            return files[i];
        }
        i++;
    }
    canfile tmp;
    tmp.data = *(new canmem((char*)"Error: file not found."));
    tmp.cachestate = -1;
    return tmp;
}

bool libcanister::canister::writeFile(canmem path, canmem data)
{
    canfile wrapper;
    wrapper.path = path;
    wrapper.data = data;
    wrapper.cfid = -1;
    wrapper.dsize = -1;
    wrapper.cachestate = 2;
    wrapper.parent = this;
    writeFile(wrapper);
    return true;
}

bool libcanister::canister::writeFile(canfile file)
{
    if (readonly)
    {
        cout << "Error: canister is read only, file could not be written." << endl;
        return false;
    }
    int i = 0;
    while (i < info.numfiles)
    {
        if (!strcmp(files[i].path.data, file.path.data))
        {
            canmem tmpdata = bzipWrapper::compress(file.data);
            if (tmpdata.size == files[i].dsize || tmpdata.size < files[i].dsize - 6)
            {
                files[i].dsize = tmpdata.size;
                files[i].data = file.data;
                files[i].cachestate = 2; //needs flush
                if (tmpdata.size < files[i].dsize - 6)
                {
                    #warning "Need to handle fragment."
                }
                return true;
            }
            else
            {
                files[i].cfid += 0xFF000000;
                files[i].isfrag = 1;
                files[i].path = *(new canmem((char*)"FRAGMENT"));
                ++info.numfiles;
            }
        }
        i++;
    }
    file.cfid = ++info.numfiles;
    file.cachestate = 2;
    canfile* newSet = new canfile[info.numfiles];
    memcpy(newSet, files, (info.numfiles - 1) * sizeof(canfile));
    newSet[info.numfiles-1] = file;
    files = newSet;
    int tLen = TOC.data.size;
    TOC.data.size += file.path.size;
    char* tmp = TOC.data.data;
    TOC.data.data = new char[TOC.data.size];
    memcpy(TOC.data.data, tmp, tLen);
    memcpy(TOC.data.data + tLen, file.path.data, file.path.size);
    TOC.data.data[TOC.data.size-1] = '\n';
    return true;
}

//dumps files from memory until cachecnt <= cachemax
//first dumps unmodified files from memory until the condition is met
//however, it will do a full cache flush if needed
void libcanister::canister::cacheclean (int sCFID, bool dFlush)
{
    int i = 0;
    while (i < info.numfiles && cachecnt > cachemax)
    {
        if (files[i].cfid != sCFID && (dFlush || files[i].cachestate == 1)) //prioritize cache dumping for
                                                                            //files which haven't changed
                                                                            //as they require least work
        {
            if (files[i].cachestate == 2)
                files[i].cachedump();
            else
            {
                files[i].data = canmem::null();
                files[i].cachestate = 0;
            }
        }
        i++;
    }
    if (!dFlush && cachecnt > cachemax && !readonly)
        cacheclean(sCFID, true);
}

libcanister::canister::~canister()
{
    close();
}

//flushes all caches and prepares for deletion
int libcanister::canister::close ()
{
    if (readonly)
        return 0;
    int i = 0;
    while (i < info.numfiles)
    {
        cout << files[i].cachestate << endl;
        if (files[i].cachestate == 2)
            files[i].cachedump();
        else
        {
            files[i].data = canmem::null();
            files[i].cachestate = 0;
        }
        i++;
    }
    #warning "Need to implement TOC output."
    readonly = true;
    return 0;
}

int libcanister::canister::open()
{
    canmem fspath = info.path;
    ifstream infile;
    infile.open(fspath.data);
    TOC.parent = this;
    TOC.path = canmem::null();
    TOC.cachestate = 1;
    TOC.cfid = -1;
    TOC.dsize = 0;
    if (!infile.is_open())
    {
        info.internalname = canmem::null();
        info.numfiles = 0;
        canmem tocData;
        tocData.data = (char*)"";
        tocData.size = 0;
        TOC.data = tocData;
        return 0;
    }
    else
    {
        //begin interpreting the file
        //lets make some temporary variables to store the header
        unsigned char temp1, temp2, temp3, temp4, temp5;
        readonly = false;
        infile >> temp1;
        infile >> temp2;
        infile >> temp3;
        infile >> temp4;
        infile >> temp5;
        //does the header match?
        if ((temp1 == 0x01) && (temp2 == 'c') && (temp3 == 'a') && (temp4 == 'n') && (temp5 == 0x01))
        {   //yes, valid header
            dout << "valid header" << endl;
            //read in the number of files
            info.numfiles = readint32(infile);
            dout << "numfiles: " << info.numfiles << endl;
            int footerloc = readint32(infile);
            infile.seekg(footerloc, ios::beg);
            //does the footer match?
            infile >> temp1;
            infile >> temp2;
            infile >> temp3;
            infile >> temp4;
            infile >> temp5;
            if ((temp1 == 0x01) && (temp2 == 'c') && (temp3 == 'a') && (temp4 == 'n') && (temp5 == 0x01))
            {   //yes, valid header
                //create a file array to hold the files
                files = new canfile[info.numfiles];
                //set the internal name of the canister
                info.internalname = readstr(infile);
                dout << "internalname: " << info.internalname.data << endl;
                int i = 0;
                char* tocRaw;
                int tocLen = 0;
                infile >> temp1;
                //loop through the file headers for each file
                while (temp1 == 0x01)
                {                
                    //if we've exceeded the number of files and have found another
                    //then something fishy is happening somewhere.. let's just
                    //use the info we have and ignore the other stuff in here
                    //Entering read only mode to prevent further damage to the canister
                    if (i >= info.numfiles)
                    {
                        cerr << "Corrupted Canister! Error message: File table length exceeds the number of files!" << endl;
                        cerr << "Best recovery option: read-only" << endl;
                        cerr << "Opening as read-only to prevent further damage to the canister." << endl;
                        readonly = true;
                        //loop to the end of the file header
                        while (temp1 != 0x03)
                            infile >> temp1;
                        break;
                    }
                    
                    //read the file header
                    files[i].parent = this;
                    files[i].dsize = readint64(infile);
                    dout << "dsize: " << files[i].dsize << endl;
                    files[i].cfid = readint32(infile);
                    dout << "cfid: " << files[i].cfid << endl;
                    files[i].path = readstr(infile);
                    files[i].cachestate = 0;
                    files[i].data = canmem::null();

                    //deal with the table of contents
                    int tLen = tocLen;
                    tocLen += files[i].path.size;
                    char* tmp = tocRaw;
                    tocRaw = new char[tocLen];
                    memcpy(tocRaw, tmp, tLen);
                    memcpy(tocRaw + tLen, files[i].path.data, files[i].path.size);
                    tocRaw[tocLen-1] = '\n';
                    //dout << "tocRaw:" << endl << "\"" << tocRaw << "\"" << endl;

                    //read the next file
                    i++;
                    infile >> temp1;
                }
                if (temp1 != 0x03)
                {
                    cerr << "Corrupted Canister! Error message: Incomplete file table! Data: " << (unsigned int)temp1 << endl;
                    return -1;
                }
                canmem tocDat = *(new canmem(tocLen));
                tocDat.data = tocRaw;
                TOC.data = tocDat;

                infile.close();
                
                return 1;
            }
            else
            {
                cout << "Error: canister is corrupt." << endl;
                return -1;
            }
        }
        else
        {
            cout << "Error: canister is corrupt." << endl;
            return -1;
        }
    }
    return -1;
}

