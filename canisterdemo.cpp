#include "libcanister.h"
using namespace libcanister;
#define cStr(x) *(new canmem((char*)x))
int main()
{
    canmem location;
    location.data = (char*)"candemo";
    location.countlen();
    canister demo = *(new canister(location));
    // char theFile[1024];
    // cout << "Enter filename: ";
    // cin >> theFile;
    // canister demo = *(new canister(theFile));
    demo.open();
    canfile important = demo.getFile(cStr("/ImportantInfo"));
    important.data = cStr("This here niftyness.");
    important.path = cStr("/NotSoCoolInfo");
    demo.writeFile(important);
    cout << demo.getTOC().data.data << endl;
    while (true)
    {
	    char theFile[1024];
	    cout << "Enter filename: ";
	    cin >> theFile;
	    cout << demo.getFile(cStr(theFile)).data.data << endl;
	}
}
