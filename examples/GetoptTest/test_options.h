#ifndef TEST_OPTIONS_H_INCLUDED
#define TEST_OPTIONS_H_INCLUDED

#include <string>
#include <vector>

#include "getopt.h"
#include "elf2e32_opt.hpp"

static struct Opts options[] =
{
    {"definput", "tests\\libcryptou.def", OptionsType::EDEFINPUT},
    {"defoutput", "tests\\tmp\\out.def", OptionsType::EDEFOUTPUT},
    {"elfinput", "tests\\libcrypto.dll", OptionsType::EELFINPUT},
    {"output", "tests\\tmp\\libcrypto-2.4.5.tst.dll", OptionsType::EOUTPUT},
    {"dso", "tests\\tmp\\libcrypto{000a0000}.dso", OptionsType::EDSO},
    {"targettype", "STDDLL", OptionsType::ETARGETTYPE, TargetType::EStdDll},
    {"linkas", "libcrypto{000a0000}.dll", OptionsType::ELINKAS},
    {"uid1", "0x10000079", OptionsType::EUID1, 0x10000079},
    {"uid2", "0x20004c45", OptionsType::EUID2, 0x20004c45},
    {"uid3", "0x00000000", OptionsType::EUID3, 0x00000000},
    {"sid", "0x00000000", OptionsType::ESID, 0x00000000},
    {"vid", "0x20004c46", OptionsType::EVID, 0x20004c46},
    {"fixedaddress", "", OptionsType::EFIXEDADDRESS, 1},
    {"uncompressed", "", OptionsType::EUNCOMPRESSED, KFormatNotCompressed},
    {"compressionmethod", "inflate", OptionsType::ECOMPRESSIONMETHOD, KUidCompressionDeflate},
    {"compressionmethod", "bytepair", OptionsType::ECOMPRESSIONMETHOD, KUidCompressionBytePair},
    {"compressionmethod", "none", OptionsType::ECOMPRESSIONMETHOD, KFormatNotCompressed},
    {"heap", "0x00040000,0x01000000", OptionsType::EHEAP, 0x00040000, 0x01000000},
    {"stack", "0x00005000", OptionsType::ESTACK, 0x00005000},
    {"unfrozen", "", OptionsType::EUNFROZEN, true},
    {"ignorenoncallable", "", OptionsType::EIGNORENONCALLABLE, true},
    {"capability", "All-TCB", OptionsType::ECAPABILITY},
    {"libpath", "tests\\SDK_libs\\", OptionsType::ELIBPATH},
    {"sysdef", "Asym1,0;asym2,1;", OptionsType::ESYSDEF},
    {"log", "blabla.log", OptionsType::ELOG},
    {"dlldata", "", OptionsType::EDLLDATA, 1},
    {"dump", "", OptionsType::EDUMP, 1},
    {"dump", "hscdeit", OptionsType::EDUMP},
    {"e32input", "libcrypto-2.4.5.dll", OptionsType::EE32INPUT},
    {"priority", "Background", OptionsType::EPRIORITY, EPriorityBackground},
    {"version", "10.1", OptionsType::EVERSION, 0xa0001},
    {"callentry", "", OptionsType::ECALLENTRY, 1},
    {"fpu", "softvfp", OptionsType::EFPU, TFloatingPointType::EFpTypeNone},
    {"fpu", "vfpv2", OptionsType::EFPU, TFloatingPointType::EFpTypeVFPv2},
    {"fpu", "vfpv3", OptionsType::EFPU, TFloatingPointType::EFpTypeVFPv3},
    {"fpu", "vfpv3D16", OptionsType::EFPU, TFloatingPointType::EFpTypeVFPv3D16},

    {"codepaging", "paged", OptionsType::ECODEPAGING, Paging::PAGED},
    {"codepaging", "unpaged", OptionsType::ECODEPAGING, Paging::UNPAGED},
    {"codepaging", "default", OptionsType::ECODEPAGING, Paging::DEFAULT},
    {"datapaging", "paged", OptionsType::EDATAPAGING, Paging::PAGED},
    {"datapaging", "unpaged", OptionsType::EDATAPAGING, Paging::UNPAGED},
    {"datapaging", "default", OptionsType::EDATAPAGING, Paging::DEFAULT},
    {"paged", "", OptionsType::EPAGED, Paging::PAGED},
    {"unpaged", "", OptionsType::EUNPAGED, Paging::UNPAGED},
    {"defaultpaged", "", OptionsType::EDEFAULTPAGED, Paging::DEFAULT},

    {"excludeunwantedexports", "", OptionsType::EEXCLUDEUNWANTEDEXPORTS, 1},
    {"customdlltarget", "", OptionsType::ECUSTOMDLLTARGET, 1},
    {"filecrc", DefaultOptionalArg, OptionsType::FILECRC},
    {"filecrc", "lala.crc", OptionsType::FILECRC},
    {"debuggable", "", OptionsType::EDEBUGGABLE, 1},
    {"smpsafe", "", OptionsType::ESMPSAFE, 1},
    {"namedlookup", "", OptionsType::ENAMEDLOOKUP, 1},
    {"help", "", OptionsType::EHELP, 1},
    {"verbose", "", OptionsType::VERBOSE, 1},
    //From Belle SDK, not implemented
//    {"asm", "gas", OptionsType::, 1},
//    {"asm", "armas", OptionsType::, 1},
//    {"exportautoupdate", "", OptionsType::, 1},

//    {"", "", OptionsType::},
//    {"", "", OptionsType::, },
    {"", "", 0, 0},
};

std::vector<std::string> option_args = {"testapp.exe =)", "--definput=tests\\libcryptou.def",
 "--defoutput=tests\\tmp\\out.def", "--elfinput=tests\\libcrypto.dll", "--version=10.1",
 "--output=tests\\tmp\\libcrypto-2.4.5.tst.dll", "--dso=tests\\tmp\\libcrypto{000a0000}.dso",
 "--targettype=STDDLL", "--linkas=libcrypto{000a0000}.dll", "--uid1=0x10000079",
 "--uid2=0x20004c45", "--uid3=0x00000000", "--sid=0x00000000", "--vid=0x20004c46",
 "--fixedaddress", "--uncompressed", "--compressionmethod=inflate", "--heap=0x00040000,0x01000000",
 "--stack=0x00005000", "--unfrozen", "--ignorenoncallable", "--capability=All-TCB",
 "--libpath=tests\\SDK_libs\\", "--sysdef=Asym1,0;asym2,1;", "--log=blabla.log", "--dlldata",
 "--dump=hscdeit", "--e32input=libcrypto-2.4.5.dll", "--priority=Background", "--verbose",
 "--callentry", "--fpu=softvfp", "--paged", "--unpaged", "--defaultpaged", "--customdlltarget",
 "--filecrc", "--filecrc=lala.crc", "--debuggable", "--smpsafe", "--namedlookup"//, "--help"
 };

#endif // TEST_OPTIONS_H_INCLUDED
