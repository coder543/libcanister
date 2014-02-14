#include "libcanister.h"

libcanister::canmem* libcanister::canmem::head;
libcanister::canmem* libcanister::canmem::tail;

void libcanister::canmem::addlink()
{
    if (head == NULL)
    {
        // cout << "adding link #" << countchain() + 1 << ": " << (void*)this;
        if (data != NULL)
            // cout << " " << data;
        else
            // cout << " NULL";
        // cout << " " << size;
        // cout << endl;
        // cout << "link is head and tail" << endl;
        head = this;
        tail = this;
        next = NULL;
        prev = NULL;
        return;
    }
    else if (!isonchain(this))
    {
        // cout << "adding link #" << countchain() + 1 << ": " << (void*)this;
        if (data != NULL)
            // cout << " " << data;
        else
            // cout << " NULL";
        // cout << " " << size;
        // cout << endl;
        tail->next = this;
        prev = tail;
        tail = this;
        next = 0x0;
    }
}

bool libcanister::canmem::isonchain(canmem* test)
{
    canmem* cur = head;
    while (cur != NULL)
    {
        if (cur == test)
            return true;
        cur = cur->next;
    }
    return false;
}

int libcanister::canmem::countchain()
{
    canmem* cur = head;
    int len = 0;
    while (cur != NULL)
    {
        len++;
        cur = cur->next;
    }
    return len;
}

void libcanister::canmem::walkchain()
{
    // cout << "walking chain" << endl;
    canmem* cur = head;
    while (cur != NULL)
    {
        // cout << (void*)cur << ": " << cur->size << endl;
        cur = cur->next;
    }
}

libcanister::canmem::canmem()
{
    //cout << "constructing empty canmem" << endl;
    data = 0x0;
    size = -1;
    addlink();
}

libcanister::canmem::canmem(int allocsize)
{
    data = (char*)malloc(allocsize);
    //cout << "constructing canmem " << (void*)data << " of size " << allocsize << endl;
    size = allocsize;
    zeromem();
    addlink();
}

libcanister::canmem::canmem(char* strdata)
{
    data = strdata;
    countlen();
    data = new char[size]; //we must own the pointer.
    //cout << "constructing canmem " << (void*)data << " " << strdata << endl;
    memcpy(data, strdata, size);
    addlink();
}

libcanister::canmem::canmem(const canmem &obj)
{
    if (obj.size <= 0 || obj.data == 0)
    {
        size = 0;
        data = 0x0;
        return;
    }
    size = obj.size;
    data = new char[size];
    //cout << "copying canmem " << (void*)obj.data << " " << obj.data  << " into " << (void*)data << endl;
    memcpy(data, obj.data, size);
    addlink();
}

libcanister::canmem& libcanister::canmem::operator=(const canmem &obj)
{
    if (obj.size <= 0 || obj.data == 0)
    {
        size = 0;
        data = 0x0;
        return *this;
    }
    size = obj.size;
    data = new char[size];
    //cout << "assignement/copying canmem " << (void*)obj.data << " " << obj.data  << " into " << (void*)data << endl;
    memcpy(data, obj.data, size);
    addlink();
    return *this;
}

libcanister::canmem::~canmem()
{
    //cout << "destroying canmem " << (void*)data << " ";
    //cout << data;
    if (data != NULL)
        delete[] data;
    //cout << endl;
    data = 0x0;
    size = -1;
    if (next != NULL)
        next->prev = prev;
    if (prev != NULL)
        prev->next = next;
    prev = NULL;
    next = NULL;
    // cout << "unlinked " << (void*)this << " from chain, " << countchain() << " nodes remain." << endl;
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
    while (data[++i] != 0)
        ; //loop until the very end of this data block
     size = ++i;
}

void libcanister::canmem::trim()
{
    //handle some funkyness. (specifically, the last two bytes of the file, 0x0203, would get duplicated once.)
    if (size < 1)
        return;
    while ((data[size-1] == 0x00 || data[size-1] == 0x02 || data[size-1] == 0x03) && size > 1)
        size--;
}

libcanister::canmem* libcanister::canmem::null()
{
    return new canmem();
}