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
        EDSODUMP,
        EOUTPUT,
        EDSO,
        ELIBPATH,
        EE32INPUT,
        EHEADER,
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
    uint32_t iUid1 = 0;
    uint32_t iUid2 = 0;
    uint32_t iUid3 = 0;
    uint32_t iSid = 0;
    uint32_t iVid = 0;
    uint32_t iHeapMin = 0;
    uint32_t iHeapMax = 0;
    uint32_t iStack = 0;
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
    uint32_t iCompressionMethod = 0;
    bool iUnfrozen = false;
    bool iIgnorenoncallable = false;
    std::string iCapability;
    std::string iSysdef;
    bool iDlldata = false;
    uint16_t iPriority; // executables priority
    bool iExcludeunwantedexports = false;
    bool iCustomdlltarget = false;
    bool iNamedlookup = false;
    std::string iDefinput;
    std::string iDefoutput;
    std::string iElfinput;
    std::string iOutput;
    std::string iDso;
    bool iDSODump = false;
    std::string iLibpath; //holds path to DSO separated by ';'
    std::string iE32input;
    std::string iDump = "h";
    std::string iLog;
    uint32_t iVersion; // ex: elf2e32.exe --version
    std::string iHeader;
};

#endif // ELF2E32_OPT_HPP_INCLUDED
