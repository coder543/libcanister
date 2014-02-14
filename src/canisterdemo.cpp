#include "libcanister.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>
using namespace libcanister;
using namespace std;
#define cStr(x) canmem((char*)x)

void docmd(canister &usercan, char* rcmd)
{
    rcmd[6] = 0; //"substring" this to the first few characters only.
    char cmdbufa[512], cmdbufb[512];
    if (!strcmp(rcmd, "helpme"))
    {
        cout << "The supported commands are as follows." << endl;
        cout << "helpme: supplies this help text." << endl;
        cout << "rename: renames this canister (internally)." << endl;
        cout << "import: imports a file from outside the canister." << endl;
        cout << "export: exports a file from the canister to the outside." << endl;
        cout << "delete: deletes a file inside the canister." << endl;
        cout << "printf: prints a file from inside the canister." << endl;
        cout << "tabcon: displays a 'table of contents' for the canister." << endl;
        cout << "comrat: compression ratio, tells how compressed a file is." << endl;
        cout << "depart: quit/exit this demo program immediately (and save changes)." << endl;
    }
    else if (!strcmp(rcmd, "rename"))
    {
        cout << "the current name is: " << usercan.info.internalname.data << endl;
        cout << "give the new name: ";
        cin >> cmdbufa;
        usercan.info.internalname = canmem(cmdbufa);
        cout << "Renamed to " << cmdbufa << "!" << endl;
    }
    else if (!strcmp(rcmd, "import"))
    {
        cout << "file to import: ";
        cin >> cmdbufa;
        cout << "what should its new path be? ";
        cin >> cmdbufb;

        //read file into memory
        ifstream infile;
        infile.open(cmdbufa);
        infile.seekg(0, std::ios::end);
        int buflen = infile.tellg();
        if (buflen <= 0)
        {
            cout << "Error: file not found." << endl;
            return;
        }
        char* buffer = (char*)malloc(buflen);
        infile.seekg(0, std::ios::beg);
        infile.read(buffer, buflen);
        infile.close();
        canmem data(buflen);
        memcpy(data.data, buffer, buflen);
        canmem path(cmdbufb);
        usercan.writeFile(path, data);
        free(buffer);
        cout << "Done." << endl;
    }
    else if (!strcmp(rcmd, "export"))
    {
        cout << "file to export: ";
        cin >> cmdbufa;
        cout << "what should its new path be? ";
        cin >> cmdbufb;

        //export the file
        canfile toexport = usercan.getFile(cStr(cmdbufa));
        ofstream outfile;
        outfile.open(cmdbufb);
        outfile.write(toexport.data.data, toexport.data.size);
        outfile.close();
        cout << "Done." << endl;
    }
    else if (!strcmp(rcmd, "delete"))
    {
        cout << "file to delete: ";
        cin >> cmdbufa;
        usercan.delFile(cStr(cmdbufa));
        cout << "Done." << endl;
    }
    else if (!strcmp(rcmd, "printf"))
    {
        cout << "which file? ";
        cin >> cmdbufa;
        cout << usercan.getFile(cStr(cmdbufa)).data.data << endl;
    }
    else if (!strcmp(rcmd, "tabcon"))
        cout << usercan.getTOC().data.data << endl;
    else if (!strcmp(rcmd, "comrat"))
    {
        cout << "file to check: ";
        cin >> cmdbufa;

        canfile tocheck = usercan.getFile(cStr(cmdbufa));
        cout << "compression ratio is " << (float)tocheck.data.size/(float)tocheck.dsize << endl;
    }
    else if (!strcmp(rcmd, "depart"))
    {
        usercan.close();
        exit(0);
    }
    else
    {
        cout << "command not recognized." << endl;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cout << "you must specify a canister file." << endl;
        cout << "./canidemo CANISTER" << endl;
        cout << "if the canister does not exist, it will be created." << endl;
        exit(2);
    }
    canmem location(argv[1]);
    // location.data = argv[1];
    // location.countlen();
    canister usercan(location);
    usercan.open();
    cout << usercan.info.internalname.data << " is now open." << endl;
    cout << "Type 'helpme' for help." << endl;
    while (true)
    {
        char cmd[12];
        cout << "canterm$ ";
        cin >> cmd;
        docmd(usercan, cmd);
    }
    usercan.close(); //this line won't ever be the one to close it
                     //but it is here as a reminder.
    canmem::walkchain();
}
