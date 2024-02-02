// Copyright (c) 2023-2024 Strizhniou Fiodar
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
// Check and fix wrong build options before artifact build starts.
//
//

#include <vector>
#include <cstdint>
#include <iomanip>

#include "common.hpp"
#include "e32common.h"
#include "elf2e32_opt.hpp"
#include "elf2e32_version.hpp"

void FixHeaderName(Args* fix)
{
    if(fix->iHeader.empty())
        return;

    std::string headerName;
    if(!fix->iElfinput.empty())
        headerName = fix->iElfinput;
    else if(!fix->iOutput.empty())
        headerName = fix->iOutput;
    else if(!fix->iDso.empty())
        headerName = fix->iDso;
    else if(!fix->iDefinput.empty())
        headerName = fix->iDefinput;
    else if(!fix->iDefoutput.empty())
        headerName = fix->iDefoutput;
    else
        ReportError(ErrorCodes::UNKNOWNHEADERNAME);
    fix->iHeader = headerName;
    fix->iHeader += ".h";
}

void ValidateCaps(Args* arg)
{
    auto pos = std::string::npos;
    std::string caps = arg->iCapability;

    if(caps.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error! Default capabilities value lost!");
    if((caps[0] == '-') || (caps[0] == '+'))
        ReportError(ErrorCodes::INVALIDARGUMENT, "capability", caps);
//  If we have nonsence like All+AllFiles
    if( (caps.find("all") != caps.find("allfiles")) && (caps.find('+') < pos) && (caps.find("allfiles") == pos) )
        ReportError(ErrorCodes::INVALIDARGUMENT, "capability", caps);
}

bool IsRunnable(TargetType type)
{
    if((type == TargetType::EExe) || (type == TargetType::EExexp)
       || (type == TargetType::EStdExe))
       return true;
    return false;
}

std::string VersionAsStr(uint32_t version)
{
    if((version == 0x000a0000u) || (version == 0))
        return "{000a0000}";
    std::stringstream buf;
    buf << "{" << std::setw(8) << std::hex << std::setfill('0') << version << "}";
    return buf.str();
}

const std::string linkAsError = "Failure while reconstructing linkas option from UID3";
void ResolveLinkAsUID(Args* arg)
{
    if(arg->iUid3 == 0)
    {
        arg->iLinkasUid.erase();
        return;
    }

    if( !arg->iLinkasUid.empty() && (arg->iLinkasUid[0] == '0') &&
       ((arg->iLinkasUid[1] == 'x') || (arg->iLinkasUid[1] == 'X')) )
    {
        arg->iLinkasUid.erase(0, 2);
        arg->iLinkasUid.insert(0, 8 - arg->iLinkasUid.size(), '0');
        arg->iLinkasUid = '[' + arg->iLinkasUid + ']';
        return;
    }

    std::stringstream buf;
    buf << "[" << std::setw(8) << std::hex << std::setfill('0') << arg->iUid3 << "]";
    if(buf.bad())
        ReportError(ErrorCodes::ZEROBUFFER, linkAsError);

    arg->iLinkasUid = buf.str();
}

/** \brief Reset invalid option --linkas
 *
 * Valid option looks like: --linkas=foo{000a0000}[10011237].dll
 * It contain 4 parts:
 * 1) target name: "foo"
 * 2) target version(hex form): "{000a0000}"
 * 3) target UID3(hex form): "[10011237]"
 * 4) file extension
 *
 * If UID3 == 0 part 3 not present in builds in SDK
 * S60_5th_Edition_SDK_v1.0 for exe and dll, stdexe and stddll
 */
void ResetInvalidLINKAS(Args* arg)
{
    if(arg->iLinkas.empty())
        return;

    size_t first = arg->iLinkas.find_first_of("{");
    size_t last = arg->iLinkas.find_first_of("}");
    size_t fst = arg->iLinkas.find_first_of("[");
    size_t lst = arg->iLinkas.find_first_of("]");

    auto pos = std::string::npos;

    bool head = (first == pos) || (last == pos) || (last < first);
    bool zeroUID3 = ((last - first) != 9);
    bool nonzeroUID3 = ( ((last - first) != 9) && ((lst - fst) != 9)) ||
                (fst == pos) || (lst == pos) || (lst < fst);
    bool tail = (arg->iUid3 > 0) ? nonzeroUID3: zeroUID3;

    if(head || tail)
    {
        ReportWarning(ErrorCodes::ZEROBUFFER, "Illformed option: " + arg->iLinkas + "\n");
        ReportWarning(ErrorCodes::ZEROBUFFER, "Example: --linkas=foo{000a0000}[10011237].dll\n");
        ReportWarning(ErrorCodes::ZEROBUFFER, "Ignoring --linkas option\n\n");
        arg->iLinkas.clear();
    }
}

std::string ConstructLinkas(TargetType type, const std::string& s, const std::string& s2)
{
    std::string tmp(s);
    tmp.erase(tmp.find_last_of("."));
    if(IsRunnable(type))
        return tmp + s2 + ".exe";
    else
        return tmp + s2 + ".dll";
}

void DeduceLINKAS(Args* arg)
{
    if(arg->iLinkas.empty() && (arg->iTargettype != TargetType::EInvalidTargetType))
    {
        std::string linkas, version;
        version = VersionAsStr(arg->iVersion);
        version += arg->iLinkasUid;
        if(!arg->iElfinput.empty())
            linkas = ConstructLinkas(arg->iTargettype, arg->iElfinput, version);
        else if(!arg->iOutput.empty())
        {
            linkas = arg->iOutput;
            linkas.insert(linkas.find_last_of("."), version);
        }
        else if(!arg->iDefoutput.empty())
            linkas = ConstructLinkas(arg->iTargettype, arg->iDefoutput, version);
        else if(!arg->iDso.empty())
        {
            if(arg->iDso.find("{") != std::string::npos) //Example: --dso="libcrypto{000a0000}.dso"
                version = arg->iLinkasUid;
            linkas = ConstructLinkas(arg->iTargettype, arg->iDso, version);
        }

        arg->iLinkas = FileNameFromPath(linkas);
    }

    if(VerboseOut())
        ReportLog("Linkas option: " + arg->iLinkas + "\n");
}

//* Return entrypoint symbol for various plugins
std::string GetEcomExportName(TargetType type)
{
    switch(type)
    {
    case TargetType::EAni:
        return "_Z15CreateCAnimDllLv,1;";
    case TargetType::EFsy:
        return "CreateFileSystem,1;";
    case TargetType::ELdd:
        return "_Z19CreateLogicalDevicev,1;";
    case TargetType::EPdd:
        return "_Z20CreatePhysicalDevicev,1;";
    case TargetType::EPdl:
        return "_Z17NewPrinterDeviceLv,1;";
    case TargetType::EPlugin:
    case TargetType::EPlugin3: //fallthru
        return "_Z24ImplementationGroupProxyRi,1;";
    case TargetType::ETextNotifier2:
        return "_Z13NotifierArrayv,1;";
    case TargetType::EVar:
        return "_Z17VariantInitialisev,1;";
    case TargetType::EVar2:
        return "VariantInitialise,1;";
    default:
        ReportError(ErrorCodes::INVALIDARGUMENT, "--targettype", std::to_string(type));
    }
    ReportError(ErrorCodes::INVALIDARGUMENT, "--targettype", std::to_string(type));
    return "";
}

void CheckE32andElf(bool noElfinput, bool noE32Image)
{
    if(noElfinput)
        ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
    if(noE32Image)
        ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");
}

void WarnForNonDllUID(uint32_t UID1)
{
    if(UID1 == KDynamicLibraryUidValue)
        return;
    ReportLog("********************\n");
    ReportLog("Wrong UID1\n");
    ReportLog("Set uid1 to KDynamicLibraryUidValue\n");
    ReportLog("********************\n");
}

void WarnForNonExeUID(uint32_t UID1)
{
    if(UID1 == KExecutableImageUidValue)
        return;
    ReportLog("********************\n");
    ReportLog("Wrong UID1\n");
    ReportLog("Set uid1 to KExecutableImageUidValue\n");
    ReportLog("********************\n");
}

//< Deduced DSO location is first set argument from: elfinput or E32output or defoutput
void DeduceDSO(Args* arg)
{
    if(IsRunnable(arg->iTargettype))
        return;
    if(!arg->iDso.empty())
        return;

    std::string path;
    if(!arg->iElfinput.empty())
        path = arg->iElfinput;
    else if(!arg->iOutput.empty())
        path = arg->iOutput;
    else if(!arg->iDefoutput.empty())
        path = arg->iDefoutput;
    if(path.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Failed to deduce dso output in DeduceDSO()\n");
    std::size_t found = path.find_last_of("/\\");
    path = path.substr(0 ,found+1);

    arg->iDso = arg->iLinkas;
    size_t pos = arg->iLinkas.find_first_of("[");
    if(pos == std::string::npos)
        pos = arg->iLinkas.find_first_of(".");
    if(pos == std::string::npos)
        ReportError(ErrorCodes::ZEROBUFFER, "Illformed option linkas in DeduceDSO(): " + arg->iLinkas + "\n");

    arg->iDso.erase(pos);
    arg->iDso += ".dso";
    arg->iDso = path + arg->iDso;
    ReportLog("Deduced DSO: " + arg->iDso + "\n");
}

void ValidateStackSize(Args* arg)
{
    if(arg->iStack == 0)
    {
        arg->iStack = KDefaultStackSize;
        return;
    }
    if(arg->iStack <= KDefaultStackSizeMax)
        return;
    ReportLog("Stack overflow: 0x%x. Reset to max: 0x%x.\n", arg->iStack, KDefaultStackSizeMax);
    arg->iStack = KDefaultStackSizeMax;
}

void ValidateDeducedLinkas(Args* arg)
{
#if SET_COMPILETIME_LOAD_EXISTED_FILECRC
    if(arg->iLinkas.empty())
        return;
    Args tmp;
    tmp.iUid3 = arg->iUid3;
    tmp.iVersion = arg->iVersion;
    tmp.iLinkasUid = arg->iLinkasUid;
    tmp.iTargettype = arg->iTargettype;
    tmp.iElfinput = arg->iElfinput;
    tmp.iOutput = arg->iOutput;
    tmp.iDefoutput = arg->iDefoutput;
    tmp.iDso = arg->iDso;

    ResolveLinkAsUID(&tmp);
    ResetInvalidLINKAS(&tmp);
    DeduceLINKAS(&tmp);
    if(arg->iLinkas != tmp.iLinkas)
        ReportError(ErrorCodes::ZEROBUFFER, "Deduced linkas invalid!"
                    "Expected: " + arg->iLinkas + " Got: " + tmp.iLinkas + "\n");
    else
        ReportLog("Deduced linkas valid!\n");
#endif // SET_COMPILETIME_LOAD_EXISTED_FILECRC
}

/** \brief Verifies and correct wrong input options
 * This function correct multiple conflict opions
 * like --datapaging with different params,
 * also fix wrong uid1 for exe and dll
 */
void ValidateOptions(Args* arg)
{
    ValidateStackSize(arg);
    FixHeaderName(arg);

    ValidateCaps(arg);

    if((arg->iDebuggable) && !IsRunnable(arg->iTargettype))
    {
        arg->iDebuggable = false;
        if(VerboseOut())
            ReportLog("--debuggable option allowed for EXE's only!\n");
    }

    if(arg->iUnfrozen)
        arg->iDefinput.clear();

    bool hasDefinput = !arg->iDefinput.empty();
    bool noDefOut = arg->iDefoutput.empty();
    bool noElfinput = arg->iElfinput.empty();
    bool noE32Image = arg->iOutput.empty();
    TargetType targetType = arg->iTargettype;

    if(!arg->iHeader.empty() && !noElfinput)
        return;

// convert .dso to .def
    if(!noElfinput && !noDefOut && noE32Image)
    {
        arg->iDSODump = true;
        return;
    }

    if(targetType == TargetType::EInvalidTargetType || targetType == TargetType::ETargetTypeNotSet)
    {
        if(hasDefinput)
            arg->iTargettype = TargetType::EImportLib;
// handle special Custom target
        else if(!noDefOut || !arg->iDso.empty())
            arg->iTargettype = TargetType::EDll;
        else
            arg->iTargettype = TargetType::EExe;
        targetType = arg->iTargettype;
    }

    if(targetType == TargetType::EInvalidTargetType || targetType == TargetType::ETargetTypeNotSet)
        ReportWarning(ErrorCodes::NOREQUIREDOPTION, "--targettype");

    ValidateDeducedLinkas(arg);
    ResolveLinkAsUID(arg);
    ResetInvalidLINKAS(arg);
    DeduceLINKAS(arg);

    DeduceDSO(arg);

    if((targetType == TargetType::EPlugin) && arg->iSysdef.empty())
        arg->iSysdef = GetEcomExportName(targetType);

    if(hasDefinput && noElfinput)
        arg->iTargettype = TargetType::EImportLib;

    uint32_t UID1 = arg->iUid1, UID2 = arg->iUid2, UID3 = arg->iUid3;
    if(!arg->iSid)
        arg->iSid = UID3;

    switch(arg->iTargettype)
    {
    case TargetType::ETargetTypeNotSet:
        break;
    case TargetType::EInvalidTargetType:
        break;
    case TargetType::EImportLib:
        if(!hasDefinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--definput");
        if(arg->iDso.empty())
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--dso");
        break;

    case TargetType::EStdDll:
        CheckE32andElf(noElfinput, noE32Image);
        WarnForNonDllUID(UID1); //< guard against wrong uids
        arg->iUid1 = KDynamicLibraryUidValue;
        arg->iUid2 = KSTDTargetUid2Value;
        break;

    case TargetType::EDll:
        CheckE32andElf(noElfinput, noE32Image);
        WarnForNonDllUID(UID1); //< guard against wrong uids
        arg->iUid1 = KDynamicLibraryUidValue;
        break;

    case TargetType::EExe:
        CheckE32andElf(noElfinput, noE32Image);
        WarnForNonExeUID(UID1);
        arg->iUid1 = KExecutableImageUidValue;
        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;

    case TargetType::EAni:
    case TargetType::EFep: //fallthru
    case TargetType::EFsy: //fallthru
    case TargetType::ELdd: //fallthru
    case TargetType::EPlugin: //fallthru
    case TargetType::EPlugin3: //fallthru
    case TargetType::EPdd: //fallthru
    case TargetType::EPdl: //fallthru
    case TargetType::ETextNotifier2: //fallthru
        if(noE32Image && noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput ""--output");
        CheckE32andElf(noElfinput, noE32Image);
        WarnForNonDllUID(UID1);
        arg->iUid1 = KDynamicLibraryUidValue;
        if(!UID2)
        {
            ReportLog("********************\n");
            ReportLog("Wrong UID2\n");
            ReportLog("Set UID2 to %u\n", arg->iTargettype);
            ReportLog("********************\n");
        }
        arg->iUid2 = arg->iTargettype;
        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;
    case EExexp:
        CheckE32andElf(noElfinput, noE32Image);
        if(noDefOut)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--defoutput");
        WarnForNonExeUID(UID1);
        arg->iUid1 = KExecutableImageUidValue;
        if(!UID2) ReportLog("Missed --uid2 option!\n");
        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;

    case EStdExe:
        CheckE32andElf(noElfinput, noE32Image);
        WarnForNonExeUID(UID1);
        arg->iUid1 = KExecutableImageUidValue;
        arg->iUid2 = KSTDTargetUid2Value;
        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;

    default:
        break;
    }
}
