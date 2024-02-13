// Copyright (c) 2019-2024 Strizhniou Fiodar
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
#include <vector>
#include "e32common.h"
#include "cmdlineprocessor.h"

const std::string DefaultOptionalArg = "none";

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
        EHEADER,
        // info for E32 image
        EDUMP,
        // common options
        ELOG,
        EVERSION,
        EMAN,
        EMANEDIT,
        EMANBUILD,
        EMANDSODUMP,
        EMANARTIFACTS,
        EHELP,
        // dev options
        FILECRC,
        TIME,
        VERBOSE,
        FORCEE32BUILD,
        // ignored
        EMESSAGEFILE,
        EDUMPMESSAGEFILE,
        // Nokia_Symbian_Belle_SDK_v1.0
        EASM,
        EE32TRAN,
        EEXPORTAUTOUPDATE,
        // error codes
        EMISSEDARG,
        ENOTRECOGNIZEDARG
    };
};

struct Args
{
    uint32_t iUid1 = 0;
    uint32_t iUid2 = 0;
    uint32_t iUid3 = 0;
    std::string iLinkasUid; // use only in hex form without prefix "0x" in brackets
    uint32_t iSid = 0;
    uint32_t iVid = 0;
    uint32_t iHeapMin = KHeapCommittedSize;
    uint32_t iHeapMax = KHeapReservedSize;
    uint32_t iStack = KDefaultStackSize;
    bool iFixedaddress = false;
    bool iCallentry = true; // callentry flag always on!
    uint32_t iFpu = TFloatingPointType::EFpTypeNone; //softfp
    Paging iCodePaging = Paging::DEFAULT;
    Paging iDataPaging = Paging::DEFAULT;
    bool iDebuggable = false; // SDK says it on for debug EXEs only, but debug DLLs has on too
    bool iSmpsafe = false;
    TargetType iTargettype = TargetType::EInvalidTargetType;
    std::string iLinkas;
    uint32_t iCompressionMethod = KUidCompressionDeflate;
    bool iUnfrozen = false;
    bool iIgnorenoncallable = false;
    std::string iCapability = "NONE";
    std::string iSysdef;
    bool iNoDlldata = true; //on by default
    uint16_t iPriority = (uint16_t)TProcessPriority::EPriorityForeground; // executables priority
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
    uint32_t iVersion = 0x000a0000u; // ex: elf2e32.exe --version
    std::string iHeader;
    uint32_t iTime[2] = {0};
    std::vector<std::string> iFileCrc;
    bool iForceE32Build = false;
};

#endif // ELF2E32_OPT_HPP_INCLUDED
