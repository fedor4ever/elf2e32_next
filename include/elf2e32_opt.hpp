// Copyright (c) 2019 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Contributors:
//
// Description:
// Service structures for ArgParser class
//
//
#ifndef ELF2E32_OPT_HPP_INCLUDED
#define ELF2E32_OPT_HPP_INCLUDED

#include <string>
#include <getopt.h>

struct OptionsType
{
    enum
    {
        EUID1,
        EUID2,
        EUID3,
        ESID,
        EVID,
        EHEAP,
        ESTACK,
        // for E32ImageHeader::iFlags
        EFIXEDADDRESS,
        ECALLENTRY,
        EFPU,
        ECODEPAGING,
        EDATAPAGING,
        EPAGED,
        EUNPAGED,
        EDEFAULTPAGED,
        EDEBUGGABLE,
        ESMPSAFE,
        // for image generation
        ETARGETTYPE,
        ELINKAS,
        EUNCOMPRESSED,
        ECOMPRESSIONMETHOD,
        EUNFROZEN,
        EIGNORENONCALLABLE,
        ECAPABILITY,
        ESYSDEF,
        EDLLDATA,
        EPRIORITY,
        EEXCLUDEUNWANTEDEXPORTS,
        ECUSTOMDLLTARGET,
        ENAMEDLOOKUP,
        // input files
        EDEFINPUT,
        EDEFOUTPUT,
        EELFINPUT,
        EOUTPUT,
        EDSO,
        ELIBPATH,
        EE32INPUT,
        // info for E32 image
        EDUMP,
        // common options
        ELOG,
        EMESSAGEFILE,
        EDUMPMESSAGEFILE,
        EVERSION,
        EHELP
    };
};

static struct option long_opts[] =
{
    {"uid1",  required_argument, 0, OptionsType::EUID1},
    {"uid2",  required_argument, 0, OptionsType::EUID2},
    {"uid3",  required_argument, 0, OptionsType::EUID3},
    {"sid",   required_argument, 0, OptionsType::ESID},
    {"vid",   required_argument, 0, OptionsType::EVID},
    {"heap",  required_argument, 0, OptionsType::EHEAP},
    {"stack", required_argument, 0, OptionsType::ESTACK},
    // for E32ImageHeader::iFlags
    {"fixedaddress",     no_argument, 0, OptionsType::EFIXEDADDRESS},
    {"callentry",        no_argument, 0, OptionsType::ECALLENTRY},
    {"fpu",        required_argument, 0, OptionsType::EFPU},
    {"codepaging", required_argument, 0, OptionsType::ECODEPAGING},
    {"datapaging", required_argument, 0, OptionsType::EDATAPAGING},
    {"paged",            no_argument, 0, OptionsType::EPAGED},
    {"defaultpaged",     no_argument, 0, OptionsType::EDEFAULTPAGED},
    {"unpaged",          no_argument, 0, OptionsType::EUNPAGED},
    {"debuggable",       no_argument, 0, OptionsType::EDEBUGGABLE},
    {"smpsafe",          no_argument, 0, OptionsType::ESMPSAFE},
    // for image generation
    {"targettype",   required_argument, 0, OptionsType::ETARGETTYPE},
    {"linkas",       required_argument, 0, OptionsType::ELINKAS},
    {"uncompressed",       no_argument, 0, OptionsType::EUNCOMPRESSED},
    {"compressionmethod", required_argument, 0, OptionsType::ECOMPRESSIONMETHOD},
    {"unfrozen",           no_argument, 0, OptionsType::EUNFROZEN},
    {"ignorenoncallable",  no_argument, 0, OptionsType::EIGNORENONCALLABLE},
    {"capability",   required_argument, 0, OptionsType::ECAPABILITY},
    {"sysdef",       required_argument, 0, OptionsType::ESYSDEF},
    {"dlldata",            no_argument, 0, OptionsType::EDLLDATA},
    {"priority",     required_argument, 0, OptionsType::EPRIORITY},
    {"excludeunwantedexports",   no_argument, 0, OptionsType::EEXCLUDEUNWANTEDEXPORTS},
    {"customdlltarget",    no_argument, 0, OptionsType::ECUSTOMDLLTARGET},
    {"namedlookup",        no_argument, 0, OptionsType::ENAMEDLOOKUP},
    // input files
    {"definput",  required_argument, 0, OptionsType::EDEFINPUT},
    {"defoutput", required_argument, 0, OptionsType::EDEFOUTPUT},
    {"elfinput",  required_argument, 0, OptionsType::EELFINPUT},
    {"output",    required_argument, 0, OptionsType::EOUTPUT},
    {"dso",       required_argument, 0, OptionsType::EDSO},
    {"libpath",   required_argument, 0, OptionsType::ELIBPATH},
    {"e32input",  required_argument, 0, OptionsType::EE32INPUT},
    // info for E32 image
    {"dump",      required_argument, 0, OptionsType::EDUMP},
    // common options
    {"log",             required_argument, 0, OptionsType::ELOG},
    {"version",               no_argument, 0, OptionsType::EVERSION},
    {"help",                  no_argument, 0, OptionsType::EHELP},
    // ignored options
    {"messagefile",     required_argument, 0, OptionsType::EMESSAGEFILE},
    {"dumpmessagefile", required_argument, 0, OptionsType::EDUMPMESSAGEFILE},
    {0,0,0,0}
};

struct Args
{
    std::string iUid1;
    std::string iUid2;
    std::string iUid3;
    std::string iSid;
    std::string iVid;
    std::string iHeap; // holds max and min Heap size separated by ','
    std::string iStack;
    bool iFixedaddress = false;
    bool iCallentry = false;
    std::string iFpu;
    bool iCodepaging = false;
    bool iDatapaging = false;
    bool iPaged = false;
    bool iUnpaged = false;
    bool iDefaultpaged = false;
    bool iDebuggable = false;
    bool iSmpsafe = false;
    std::string iTargettype;
    std::string iLinkas;
    bool iUncompressed = false;
    std::string iCompressionmethod;
    bool iUnfrozen = false;
    bool iIgnorenoncallable = false;
    std::string iCapability;
    std::string iSysdef;
    bool iDlldata = false;
    std::string iPriority;
    bool iExcludeunwantedexports = false;
    bool iCustomdlltarget = false;
    bool iNamedlookup = false;
    std::string iDefinput;
    std::string iDefoutput;
    std::string iElfinput;
    std::string iOutput;
    std::string iDso;
    std::string iLibpath; //holds path to DSO separated by ';'
    std::string iE32input;
    std::string iDump;
    std::string iLog;
    std::string iVersion; // ex: elf2e32.exe --version=22.06.1982
};

#endif // ELF2E32_OPT_HPP_INCLUDED
