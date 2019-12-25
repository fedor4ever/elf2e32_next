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
    std::string iDump = "h";
    std::string iLog;
    std::string iVersion; // ex: elf2e32.exe --version
};

#endif // ELF2E32_OPT_HPP_INCLUDED
