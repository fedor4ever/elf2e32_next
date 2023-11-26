#include <sstream>
#include <iostream>
#include "common.hpp"
#include "elf2e32_opt.hpp"

void ValidateOptions(Args* arg);
using namespace std;

const uint32_t uid3 = 0xe9e357a1;
//uint32_t sid = ;
//uint32_t fpu = ;
const uint32_t version_default = 0x000a0000u;

string dso("stddll_example{000a0000}.dso");
const string linkas_uid3("stddll_example{000a0000}[e9e357a1].dll");
const string output("epoc_stddll_example.dll");
const string defoutput("stddll_example{000a0000}.def");
const string sysdef("_Z24ImplementationGroupProxyRi,1;");

const string elfinput("stddll_example.dll");

bool VerboseOut() {return true;}

template < typename T >
void ThrowIfNotMatch(const T& src, const T& target, const string& msg)
{
    if(target != src){
        stringstream tmp("Expected result for target ");
        tmp << msg << ": " << target << " but has: " << src << "\n";
        ReportError(ErrorCodes::ZEROBUFFER, tmp.str());
    }
}

int main()
{
    cout << "Tests for deduced options!" << endl;
    try
    {
        Args arg;
        arg.iTargettype = TargetType::EPlugin;
        arg.iUid3 = uid3;
        arg.iOutput = output;
        arg.iElfinput = elfinput;
        ValidateOptions(&arg);

        ThrowIfNotMatch(arg.iUid1, KDynamicLibraryUidValue, "uid1\n");
        ThrowIfNotMatch(arg.iUid2, (uint32_t)TargetType::EPlugin, "uid2\n");
        ThrowIfNotMatch(arg.iUid3, uid3, "uid3\n");
        ThrowIfNotMatch(arg.iSid, uid3, "sid\n");
        ThrowIfNotMatch(arg.iDso, dso, "dso\n");
        ThrowIfNotMatch(arg.iOutput, output, "output\n");
        ThrowIfNotMatch(arg.iElfinput, elfinput, "elfinput\n");
        ThrowIfNotMatch(arg.iFpu, (uint32_t)TFloatingPointType::EFpTypeNone, "softfp\n");
        ThrowIfNotMatch(arg.iVersion, version_default, "version\n");
        ThrowIfNotMatch(arg.iLinkas, linkas_uid3, "linkas\n");
        ThrowIfNotMatch(arg.iSysdef, sysdef, "sysdef\n");
//        ThrowIfNotMatch(arg.i, , "\n");
        ReportLog("All tests passed!");
        return 0;
    }catch(ErrorCodes err){
        return -err;
    }catch(...){
        ReportWarning(ErrorCodes::UNKNOWNERROR);
        return -ErrorCodes::UNKNOWNERROR;
    }
        ReportLog("Oops! Some tests failed!");
    return 0;
}
