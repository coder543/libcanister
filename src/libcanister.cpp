#include "libcanister.h"
#include "fileinterpretation.h"
#include "bzipWrapper.h"
using namespace std;

/*
unhandled edge cases for fragments
-- creating a new fragment adjacent to an existing one
   |-- should rewrite the existing fragment to encompass both
-- defragging
*/

libcanister::canister::canister (char* fspath)
{
    info.path = canmem(fspath);
    cachemax = 25;
    cachecnt = 0;
    readonly = false;
}

libcanister::canister::canister (canmem fspath)
{
    info.path = *(new canmem(fspath)); //manually invoke copy constructor to avoid elision damage
    cachemax = 25;
    cachecnt = 0;
    readonly = false;
}

libcanister::canfile libcanister::canister::getTOC()
{
    return TOC;
}

int libcanister::canister::delFile(canmem path)
{
    int i = 0;
    //search for the file
    while (i < info.numfiles)
    {
        //if we found it
        if(!strcmp(files[i].path.data, path.data))
        {
            files[i].data = *(new canmem(files[i].dsize));
            files[i].data.fragmem();
            files[i].cachestate = 2;
            files[i].isfrag = 1;
            files[i].cfid |= 0x80000000;
            files[i].path.data = (char*)"FRAGMENT";
            files[i].path.countlen();
            return 0;
        }
        i++;
    }
    return -1;
}

libcanister::canfile libcanister::canister::getFile(canmem path)
{
    int i = 0;
    //search for the file
    while (i < info.numfiles)
    {
        //if we found it
        if(!strcmp(files[i].path.data, path.data))
        {
            //pull it into memory
            files[i].cache();
            //and return it
            return files[i];
        }
        i++;
    }
    //otherwise generate an error file to report it
    canfile tmp;
    tmp.data = *(new canmem((char*)"Error: file not found."));
    tmp.cachestate = -1;
    return tmp;
}

bool libcanister::canister::writeFile(canmem path, canmem data)
{
    //here we simply wrap the information we're given and call
    //the 'real' writeFile function
    canfile wrapper;
    wrapper.path = path;
    wrapper.data = data;
    wrapper.cfid = -1;
    wrapper.dsize = -1;
    wrapper.isfrag = 0;
    wrapper.cachestate = 2;
    wrapper.parent = this;
    return writeFile(wrapper);;
}

bool libcanister::canister::writeFile(canfile file)
{
    //if we're in readonly mode, we're definitely not going to be
    //writing this file under any circumstance.
    if (readonly)
    {
        cout << "Error: canister is read only, file could not be written." << endl;
        return false;
    }
    //we want to first figure out how much disk space it needs
    canmem* tmpdata = bzipWrapper::compress(file.data);
    int i = 0;
    //search for the file
    dout << "writeFile([name: '" << file.path.data << "', size: " << tmpdata->size << ")" << endl;
    while (i < info.numfiles)
    {
        //if it already exists
        dout << file.path.data << " == " << files[i].path.data << " ?" << endl;
        // dout << file.data.data << endl;
        if (!strcmp(files[i].path.data, file.path.data))
        {
            dout << "writeFile -- file already exists" << endl;
            //then we check to make sure the new version would fit within the old
            if (tmpdata->size == files[i].dsize || tmpdata->size < files[i].dsize - 6)
            {
                dout << "writeFile -- new contents will fit within old." << endl;
                //whereupon we make the changeover
                files[i].dsize = tmpdata->size;
                files[i].data = file.data;
                files[i].cachestate = 2; //needs flush
                files[i].cfid &= 0xEFFFFFFF;
                if (tmpdata->size != files[i].dsize)
                {
                    //and handle the fragmented leftovers
                    //create a fragment file
                    canfile fragment;
                    fragment.cfid = (++info.numfiles) | 0x80000000;
                    fragment.cachestate = 2;
                    fragment.isfrag = 1;
                    fragment.data = *(new canmem(files[i].dsize - tmpdata->size - 5));
                    fragment.data.fragmem();
                    //then we rebuild the array to be one larger
                    canfile* newSet = new canfile[info.numfiles];
                    //copy the old into the new
                    memcpy(newSet, files, (i+1) * sizeof(canfile));
                    //put this file into the array
                    newSet[i+1] = fragment;
                    memcpy(newSet + (i+2) * sizeof(canfile), files + (i+1)*sizeof(canfile), (info.numfiles-i-1) * sizeof(canfile));
                    //and move the old pointer to look at this new array
                    files = newSet;
                }
                return true;
            }
            else
            {
                dout << "writeFile -- new file won't fit within old, making fragment." << endl;
                //otherwise, we're just going to have to make the original
                //file into one big fragment and let the rest of the
                //function take care of the bigger version of the file
                files[i].cfid |= 0x80000000;
                files[i].isfrag = 1;
                files[i].path = *(new canmem((char*)"FRAGMENT"));
                //++info.numfiles; not sure if this was needed
                break;
            }
        }
        i++;
    }
    i = 0;
    //search for a fragment large enough to hold this
    while (i < info.numfiles)
    {
        //is this a fragment?
        if (files[i].cfid & 0x80000000)
        {
            dout << "writeFile -- found a fragment of length " << files[i].dsize << endl;
            //can we fit it inside the fragment?
            if (files[i].dsize >= tmpdata->size + 6 || files[i].dsize == tmpdata->size)
            {
                dout << "writeFile -- file will fit within fragment, inserting." << endl;
                files[i] = file;
                files[i].data = *tmpdata;
                files[i].dsize = tmpdata->size;
                files[i].cachestate = 2;
                if (files[i].dsize != tmpdata->size)
                {
                    //create a fragment file
                    canfile fragment;
                    fragment.cfid = (++info.numfiles) | 0x80000000;
                    fragment.cachestate = 2;
                    fragment.isfrag = 1;
                    fragment.data = *(new canmem(files[i].dsize - tmpdata->size - 5));
                    fragment.data.fragmem();
                    //then we rebuild the array to be one larger
                    canfile* newSet = new canfile[info.numfiles];
                    //copy the old into the new
                    memcpy(newSet, files, (i+1) * sizeof(canfile));
                    //put this file into the array
                    newSet[i+1] = fragment;
                    memcpy(newSet + (i+2) * sizeof(canfile), files + (i+1)*sizeof(canfile), (info.numfiles-i-1) * sizeof(canfile));
                    //and move the old pointer to look at this new array
                    files = newSet;
                }
                //and here we add the new file onto the autogenerated TOC
                if (TOC.data.size > 0 && TOC.data.data[TOC.data.size-1] == 0x00)
                    TOC.data.size--;
                int tLen = TOC.data.size;
                TOC.data.size += file.path.size;
                char* tmp = TOC.data.data;
                TOC.data.data = new char[TOC.data.size];
                memcpy(TOC.data.data, tmp, tLen);
                memcpy(TOC.data.data + tLen, file.path.data, file.path.size);
                TOC.data.data[TOC.data.size-1] = '0';
                return true;
            }
        }
        i++;
    }
    dout << "writeFile -- creating all-new file to contain contents." << endl;
    //here we generate a new CFID for the file
    file.cfid = ++info.numfiles;
    //mark it to be in need of flushing
    file.cachestate = 2;
    //it isn't a fragment
    file.isfrag = 0;
    //then we rebuild the array to be one larger
    canfile* newSet = new canfile[info.numfiles];
    //copy the old into the new
    memcpy(newSet, files, (info.numfiles-1) * sizeof(canfile));
    //put this file into the array
    newSet[info.numfiles-1] = file;
    //and move the old pointer to look at this new array
    files = newSet;
    //and here we add the new file onto the autogenerated TOC
    if (TOC.data.size > 0 && TOC.data.data[TOC.data.size-1] == 0x00)
        TOC.data.size--;
    int tLen = TOC.data.size;
    TOC.data.size += file.path.size;
    char* tmp = TOC.data.data;
    TOC.data.data = new char[TOC.data.size];
    memcpy(TOC.data.data, tmp, tLen);
    memcpy(TOC.data.data + tLen, file.path.data, file.path.size);
    TOC.data.data[TOC.data.size-1] = '\n';
    //success!
    return true;
}

//dumps files from memory until cachecnt <= cachemax
//first dumps unmodified files from memory until the condition is met
//however, it will do a full cache flush if needed
void libcanister::canister::cacheclean (int sCFID, bool dFlush)
{
    int i = 0;
    //loop through and uncache files until we're within cachemax
    while (i < info.numfiles && cachecnt > cachemax)
    {
        //preferably uncache only files which haven't been modified
        if (files[i].cfid != sCFID && (dFlush || files[i].cachestate == 1))
        {
            if (files[i].cachestate == 2)
                files[i].cachedump();
            else
            {
                files[i].data = *canmem::null();
                files[i].cachestate = 0;
            }
        }
        i++;
    }

    //if we still haven't done enough cleaning, flush the modified files too!
    if (!dFlush && cachecnt > cachemax && !readonly)
        cacheclean(sCFID, true);
}

//actuates the close() item when naturally destroyed
libcanister::canister::~canister()
{
    close();
    int i = -1;
    while (++i < info.numfiles)
        files[i].~canfile();
    delete files;
}

//flushes all caches and prepares for object deletion
int libcanister::canister::close()
{
    //don't do anything if readonly
    if (readonly)
        return 2;

    canmem fspath = info.path;
    dout << "writing to file " << fspath.data << endl;
    fstream infile;
    infile.open(fspath.data, ios::in | ios::out | ios::binary);
    if (!infile.is_open())
    { //create new file
        fstream filechecker;
        filechecker.open(fspath.data, ios::out);
        if (!filechecker.is_open())
            return 1; // could not open file
        filechecker.close();
        infile.open(fspath.data, ios::in | ios::out | ios::binary);
    }
    infile.seekg(0, ios::beg);
    //write the header verification (c00)

    infile << (char)0x01;
    infile << 'c';
    infile << 'a';
    infile << 'n';
    infile << (char)0x01;


    //write the new file count (filect) (c01)
    unsigned char* filect = (unsigned char*)(void*)&info.numfiles;
    infile << filect[3];
    infile << filect[2];
    infile << filect[1];
    infile << filect[0];

    //make sure there is a footerloc (c11) in some form.
    infile << (unsigned char)0xFF;
    infile << (unsigned char)0xFF;
    infile << (unsigned char)0xFF;
    infile << (unsigned char)0xFF;
    infile.seekg(14, ios::beg); //seek to file section
    int i = 0;
    //otherwise, loop through the files
    while (i < info.numfiles)
    {
        //dout << files[i].cachestate << endl;
        files[i].cachedumpfinal(infile);
        files[i].data = *canmem::null();
        files[i].cachestate = 0;
        i++;
    }
    //here we rewrite the footer to represent the latest changes to the canister
    int footerloc = infile.tellg();
    // infile.seekg(footerloc, ios::beg); //reset back to footer

    //write footer verification (c00)
    infile << (char)0x01;
    infile << 'c';
    infile << 'a';
    infile << 'n';
    infile << (char)0x01;

    //c02
    info.internalname.trim();
    infile.write(info.internalname.data, info.internalname.size);
    infile << (unsigned char)0x02;

    i = 0;
    while (i < info.numfiles)
    {
        //c03
        infile << (unsigned char)0x01;

        //c04
        unsigned char* sizeout = (unsigned char*)(void*)&files[i].dsize;
        infile << sizeout[7];
        infile << sizeout[6];
        infile << sizeout[5];
        infile << sizeout[4];
        infile << sizeout[3];
        infile << sizeout[2];
        infile << sizeout[1];
        infile << sizeout[0];

        //c05
        unsigned char* id = (unsigned char*)(void*)&files[i].cfid;
        infile << id[3];
        infile << id[2];
        infile << id[1];
        infile << id[0];

        //c06
        files[i].path.trim();
        infile.write(files[i].path.data, files[i].path.size);
        infile << (unsigned char)0x02;

        i++;
    }

    infile << (unsigned char)0x03; //c07

    infile.seekg(9, ios::beg); //go back to header

    //write the new footerloc into place (c11)
    unsigned char* ftloc = (unsigned char*)(void*)&footerloc;
    infile << ftloc[3];
    infile << ftloc[2];
    infile << ftloc[1];
    infile << ftloc[0];
    infile << (unsigned char)0x03; //c07
    infile.close();

    //no other modifications to the canister should happen after this point
    //also prevents double closure from a manual close() call and the automated one
    //coming from ~canister()
    readonly = true;
    return 0;
}

int libcanister::canister::open()
{
    //here we open the file associated with this canister
    canmem fspath = info.path;
    ifstream infile;
    infile.open(fspath.data);
    //now we begin creating the table of contents
    TOC.parent = this;
    TOC.path = *canmem::null();
    TOC.cachestate = 1;
    TOC.cfid = -1;
    TOC.dsize = 0;
    //if the file didn't open, we need to do a
    //skeletal initialization of the canister
    if (!infile.is_open())
    {
        info.internalname = *(new canmem((char*)"generic canister"));
        info.numfiles = 0;
        canmem tocData((char*)"\0");
        TOC.data = tocData;
        return 0;
    }
    //otherwise, we're going to work on reading the file's
    //header and footer into memory
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
        //does the header match? (c00)
        if ((temp1 == 0x01) && (temp2 == 'c') && (temp3 == 'a') && (temp4 == 'n') && (temp5 == 0x01))
        {   //yes, valid header
            dout << "valid header" << endl;
            //read in the number of files
            info.numfiles = readint32(infile);
            dout << "numfiles: " << info.numfiles << endl;
            #warning "technically this should be a readint64, but seekg won't accept the 64-bit int. Solution needed."
            int footerloc = readint32(infile);
            infile.seekg(footerloc, ios::beg);
            //does the footer match?
            infile >> temp1;
            infile >> temp2;
            infile >> temp3;
            infile >> temp4;
            infile >> temp5;
            if ((temp1 == 0x01) && (temp2 == 'c') && (temp3 == 'a') && (temp4 == 'n') && (temp5 == 0x01))
            {   //yes, valid footer (c00)
                //create a file array to hold the files
                files = new canfile[info.numfiles];
                //set the internal name of the canister
                info.internalname = *readstr(infile);
                dout << "internalname: " << info.internalname.data << endl;
                int i = 0;
                char* tocRaw = 0x0;
                int tocLen = 0;
                infile >> temp1;
                //loop through the file headers for each file
                while (temp1 == 0x01)
                {                
                    //if we've exceeded the number of files and have found another
                    //then something fishy is happening somewhere... let's just
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
                    files[i].path = *readstr(infile);
                    files[i].cachestate = 0;
                    files[i].data = *canmem::null();
                    if (files[i].cfid & 0x80000000 || !strcmp(files[i].path.data, (char*)"FRAGMENT"))
                        files[i].isfrag = 1;
                    else
                    {
                        files[i].isfrag = 0;
                        //deal with the table of contents
                        int tLen = tocLen;
                        tocLen += files[i].path.size;
                        char* tmp = tocRaw;
                        tocRaw = new char[tocLen+1];
                        if (tLen > 0)
                            memcpy(tocRaw, tmp, tLen);
                        memcpy(tocRaw + tLen, files[i].path.data, files[i].path.size);
                        tocRaw[tocLen-1] = '\n';
                    }

                    //read the next file
                    i++;
                    infile >> temp1;
                }
                if (temp1 != 0x03)
                {
                    cerr << "Corrupted Canister! Error message: Incomplete file table! Data: " << (unsigned int)temp1 << " " << infile.tellg() << endl;
                    return -1;
                }
                if (tocLen > 0)
                    tocRaw[tocLen] = 0x00;
                else
                {
                    tocRaw = new char[1];
                    tocRaw[0] = 0x00;
                }
                TOC.data = *(new canmem(tocRaw));

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

