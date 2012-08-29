#include "libcanister.h"
using namespace libcanister;
#define cStr(x) *(new canmem((char*)x))
int main()
{
    canmem location;
    location.data = (char*)"candemocmp";
    location.countlen();
    canister demo = *(new canister(location));
    demo.open();
    canmem data = cStr("This here be some niftyness.");
    canmem path = cStr("/ANotSoCoolInfo");
    demo.writeFile(path, data);
    cout << demo.getTOC().data.data << endl;
    while (true)
    {
        char theFile[1024];
        cout << "Enter filename: ";
        cin >> theFile;
        if (theFile[0] == '/')
            cout << demo.getFile(cStr(theFile)).data.data << endl;
        else
            break;
    }
    demo.close();
}
