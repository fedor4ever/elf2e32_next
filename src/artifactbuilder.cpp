// Copyright (c) 2020 Strizhniou Fiodar
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
// Build E32Image and other companions
//
//

#include <vector>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "task.hpp"
#include "symbol.h"
#include "deffile.h"
#include "dsofile.h"
#include "common.hpp"
#include "e32common.h"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "artifactbuilder.h"
#include "symbolprocessor.h"
#include "elf2e32_version.hpp"

void FixHeaderName(Args* fix);
void MakeImportHeader(Symbols symbols, std::string dllName);
void ValidateOptions(Args* arg);
void ValidateCaps(Args* arg);
std::string ResolveLinkAsUID(const Args* arg);

ArtifactBuilder::ArtifactBuilder(Args* param): iOpts(param)
{
    //ctor
}

ArtifactBuilder::~ArtifactBuilder()
{
    //dtor
}

void ArtifactBuilder::Run()
{
    PrepareBuild();
    MakeDSO();
    MakeDef();
    MakeE32();
    MakeImportHeader(iSymbols, iOpts->iHeader);
}

void ArtifactBuilder::PrepareBuild()
{
    ValidateOptions(iOpts);
    if(!iOpts->iElfinput.empty())
    {
        iElfParser = new ElfParser(iOpts->iElfinput);
        iElfParser->GetElfFileLayout();
    }
    SymbolProcessor processor(iOpts, iElfParser);
    iSymbols = processor.GetExports();
    DsoImpLibName();
}

void ArtifactBuilder::DsoImpLibName()
{
    if(iOpts->iDefinput.empty())
        return;
    DefFile d;
    d.GetSymbols(iOpts->iDefinput.c_str());
    iDsoImpLibName = d.GetDsoImpLibName();
    if(iOpts->iDSODump)
        iDsoImpLibName = iElfParser->DsoImpLibName();
}

void ArtifactBuilder::MakeDSO()
{
    if(iOpts->iDso.empty())
        return;
    DSOFile* dso = new DSOFile();
    dso->WriteDSOFile(iOpts, iSymbols, iDsoImpLibName);
    delete dso;
}

void ArtifactBuilder::MakeDef()
{
    if(iOpts->iDefoutput.empty())
        return;
    DefFile deffile;

    std::vector<std::string> tmp;
    if(iOpts->iDSODump)
        deffile.SetDsoImpLibName(iDsoImpLibName);
    deffile.WriteDefFile(iOpts->iDefoutput.c_str(), iSymbols);
}

void ArtifactBuilder::MakeE32()
{
    if(iOpts->iOutput.empty())
        return;
    BuildE32Image(iOpts, iElfParser, iSymbols);
}

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

void MakeImportHeader(Symbols symbols, std::string dllName)
{
    if(dllName.empty())
        return;

    std::fstream fstr(dllName, std::fstream::out | std::fstream::trunc);
    ToolVersion ver;
    auto it = symbols.begin();

    fstr << "//This file generated by elf2e32 ";
    fstr << (uint32_t)ver.iMajor << "." << (uint32_t)ver.iMinor << "." << ver.iBuild;
    fstr << " to easy dynamic linking.\n";
    fstr << "//Usage example:\n// #include <e32std.h>\n// RLibrary library;\n";
    fstr << "// User::LeaveIfError(library.Load(" << dllName << "));\n";
    fstr << "// TLibraryFunction entry=library.Lookup(" << (*it)->AliasName() << ");\n";
    fstr << "// Call the function to create new CMessenger\n"
        "// CMessenger* messenger=(CMessenger*) entry();\n";
    fstr << "// library.Close();\n";

    fstr << "#ifndef ELF2E32_" << dllName << "\n";
    fstr << "#define ELF2E32_" << dllName << "\n";
    for(auto x: symbols)
    {
        fstr << "#define " << x->AliasName() << "\t" << x->Ordinal() << "\n";
    }

    fstr << "#endif // ELF2E32_" << dllName << "\n";
    fstr.close();
}

bool IsRunnable(TargetType type)
{
    if((type == TargetType::EExe) || (type == TargetType::EExexp)
       || (type == TargetType::EStdExe))
       return true;
    return false;
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
}

void WarnForNonDllUID()
{
    ReportLog("********************\n");
    ReportLog("Wrong UID1\n");
    ReportLog("Set uid1 to KDynamicLibraryUidValue\n");
    ReportLog("********************\n");
}

void WarnForNonExeUID()
{
    ReportLog("********************\n");
    ReportLog("Wrong UID1\n");
    ReportLog("Set uid1 to KExecutableImageUidValue\n");    ReportLog("********************\n");
}

void DeduceLINKAS(Args* arg)
{
    if(arg->iLinkas.empty() && (arg->iTargettype != TargetType::EInvalidTargetType))
    {
        std::string linkas;
        if(!arg->iOutput.empty())
        {
            linkas = arg->iOutput;
            linkas.insert(linkas.find_last_of("."), "{000a0000}");
            linkas.insert(linkas.find_last_of("."), arg->iLinkasUid);
        }
        else if(!arg->iDefoutput.empty())
        {
            linkas = arg->iDefoutput;
            linkas.erase(linkas.find_last_of("."));
            linkas += "{000a0000}";
            linkas += arg->iLinkasUid;
            linkas += ".dll";
        }
        else if(!arg->iDso.empty())
        {
            linkas = arg->iDso;
            linkas.erase(linkas.find_last_of("."));
            linkas += "{000a0000}";
            linkas += arg->iLinkasUid;
            linkas += ".dll";
        }
        arg->iLinkas = FileNameFromPath(linkas);
    }
}

const std::string linkAsError = "Failure while reconstructing linkas option from UID3";
std::string ResolveLinkAsUID(Args* arg)
{
    if(arg->iLinkasUid.size() > 10) ReportError(ErrorCodes::ARGUMENTNAME, arg->iLinkasUid);
    if(arg->iLinkasUid.size() == 9) ReportError(ErrorCodes::ARGUMENTNAME, arg->iLinkasUid);
    if((arg->iLinkasUid.size() == 10) && (arg->iLinkasUid[0] != '0') &&
        ((arg->iLinkasUid[1] != 'x') || (arg->iLinkasUid[1] != 'X'))) ReportError(ErrorCodes::ARGUMENTNAME, arg->iLinkasUid);
    std::stringstream buf;

//    strtoul() return zero for zero strings("0x00") and no valid conversion could be performed
//    but zero UID3 is valid
    if((arg->iLinkasUid[1] == 'x') || (arg->iLinkasUid[1] == 'x'))
    {
        buf << "[";
        size_t count = std::count_if( arg->iLinkasUid.begin(), arg->iLinkasUid.end(), []( char c ){return c == '0';});
        // first comes defaull zero hex UID
        if((count + 1) == arg->iLinkasUid.size())
        {
            buf << "00000000" << "]";
            if(buf.bad())
                ReportError(ErrorCodes::ZEROBUFFER, linkAsError);
            return buf.str();
        }
        // shertened zero hex UID
        if(arg->iLinkasUid.size() < 10)
        {
            std::string tmp;
            tmp.append(10 - arg->iLinkasUid.size(), '0');
            tmp += arg->iLinkasUid.substr(2, arg->iLinkasUid.size());
            buf << tmp << "]";
            if(buf.bad())
                ReportError(ErrorCodes::ZEROBUFFER, linkAsError);
            return buf.str();
        }
        // fully qualified hex UID
        if(arg->iLinkasUid.size() == 10)
        {
            buf << arg->iLinkasUid.substr(2, arg->iLinkasUid.size()) << "]";
            if(buf.bad())
                ReportError(ErrorCodes::ZEROBUFFER, linkAsError);
            return buf.str();
        }
        ReportError(ErrorCodes::ARGUMENTNAME, arg->iLinkasUid);
    }

    uint32_t iUid3 = strtoul(arg->iLinkasUid.c_str(), nullptr, 0);
//    no valid conversion could be performed, a zero value is returned
    if(iUid3 == 0)
        ReportError(ErrorCodes::ZEROBUFFER, "No valid conversion could be performed,"
                    " a zero value is returned while convert UID3 to digit");
    if((iUid3 == ULONG_MAX) && (errno == ERANGE))
        ReportError(ErrorCodes::ZEROBUFFER, "Value read is out of the range while convert UID3 to digit");

    buf << "[" << std::setw(8) << std::hex << std::setfill('0') << iUid3 << "]";

    if(buf.bad())
        ReportError(ErrorCodes::ZEROBUFFER, linkAsError);
    return buf.str();
}

/** \brief Verifies and correct wrong input options
 * This function correct multiple conflict opions
 * like --datapaging with different params,
 * also fix wrong uid1 for exe and dll
 */
void ValidateOptions(Args* arg)
{
    FixHeaderName(arg);

    ValidateCaps(arg);

    if((arg->iDebuggable) && !IsRunnable(arg->iTargettype))
    {
        arg->iDebuggable = false;
        ReportLog("--debuggable option allowed for EXE's only!\n");
    }

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

    if(!hasDefinput && arg->iNamedlookup)
        ReportLog("Note: if option \"--namedlookup\" supplied and \"--definput\" omitted E32"
                "image generated that tool differs from SDK tool output for code relocs"
                "section. Correctness unknown.\n");

    if(targetType == TargetType::EInvalidTargetType || targetType == TargetType::ETargetTypeNotSet)
    {
        ReportWarning(ErrorCodes::NOREQUIREDOPTION, "--targettype");
        if(hasDefinput)
            arg->iTargettype = TargetType::EImportLib;
// handle special Custom target
        else if(!noDefOut || !arg->iDso.empty())
            arg->iTargettype = TargetType::EDll;
        else
            arg->iTargettype = TargetType::EExe;
        targetType = arg->iTargettype;
    }

    arg->iLinkasUid = ResolveLinkAsUID(arg);
    DeduceLINKAS(arg);

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
        if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
        if(noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");

        if(UID1 != KDynamicLibraryUidValue) //< guard against wrong uids
            WarnForNonDllUID();
        arg->iUid1 = KDynamicLibraryUidValue;
        arg->iUid2 = KSTDTargetUid2Value;

        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;
    case TargetType::EDll:
        if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
        if(noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");

        if(UID1 != KDynamicLibraryUidValue) //< guard against wrong uids
            WarnForNonDllUID();
        arg->iUid1 = KDynamicLibraryUidValue;
        if(!UID2)
        {
            ReportLog("********************\n");
            ReportLog("missed value for UID2\n");            ReportLog("********************\n");
        }
        if(arg->iTargettype == TargetType::EStdDll) arg->iUid2 = KSTDTargetUid2Value; // only that uid2 accepted for STDDLL & STDEXE
        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;
    case TargetType::EExe:
        if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
        if(noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");
        if(UID1 != KExecutableImageUidValue)
            WarnForNonExeUID();
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
        if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
        if(noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");
        if(UID1 != KDynamicLibraryUidValue)
            WarnForNonDllUID();
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
        if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
        if(noDefOut)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--defoutput");
        if(noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");

        if(UID1 != KExecutableImageUidValue)
            WarnForNonExeUID();
        arg->iUid1 = KExecutableImageUidValue;
        if(!UID2) ReportLog("Missed --uid2 option!\n");
        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;
    case EStdExe:
        if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
        if (noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");
        if(UID1 != KExecutableImageUidValue)
            WarnForNonExeUID();
        arg->iUid1 = KExecutableImageUidValue;
        arg->iUid2 = KSTDTargetUid2Value;

        if(!UID3) ReportLog("Missed --uid3 option!\n");
        break;
    default:
        break;
    }
}

void ValidateCaps(Args* arg)
{
    std::string caps = ToLower(arg->iCapability);

    if(caps.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error! Default capabilities value lost!");
    if((caps[0] == '-') || (caps[0] == '+'))
        ReportError(ErrorCodes::INVALIDARGUMENT, "capability", caps);
    if( (caps.substr(0, 3) == "all") && (caps.find('+') < std::string::npos) )
        ReportError(ErrorCodes::INVALIDARGUMENT, "capability", caps);
}
