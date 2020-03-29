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

enum Paging
{
    PAGED,
    UNPAGED,
    DEFAULT
};

//! for details see _secure_trgtype.pm in Symbian sources
enum TargetType
{
	ETargetTypeNotSet = - 2,
	EInvalidTargetType = - 1,
	/** Target type is import Library */
	EImportLib,
	/** Target type is DLL */
	EDll,
	EExe,
	EKDll,
	EVar,  // exports "_Z17VariantInitialisev"
	EVar2, // exports "VariantInitialise"
	EExexp,
	EStdExe, //UID1 - 0x1000007a UID2 - 0x20004C45
	EStdDll, //UID1 - 0x10000079 UID2 - 0x20004C45
	//ecom plugins
	/// TODO (Administrator#1#03/29/20): set right export at ord1
	EAni = 0x10003b22, // animation plugin and uid2
	EFep = 0x10005e32, // front-end processor plugin and uid1
	EFsy = 0x100039df, // file system plugin and uid2
	ELdd = 0x100000af, // logical device driver with uid2 and exports "_Z19CreateLogicalDevicev,1"
	EPlugin = 0x10009D8D, // ECom plug-in and uid2
	EPlugin3 = 0x10009D93, // ECom plug-in and uid2
	EPdd = 0x100039d0, // physical device driver with uid2 and exports "_Z20CreatePhysicalDevicev,1"
	EPdl = 0x10003b1c, // printer device plugin and uid2
	ETextNotifier2 = 0x101fe38b, // notifier for text window server plugin and uid2
	EFalseTartget
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
    Paging iCodePaging = Paging::DEFAULT;
    Paging iDataPaging = Paging::DEFAULT;
    bool iDebuggable = false;
    bool iSmpsafe = false;
    TargetType iTargettype = TargetType::EInvalidTargetType;
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
