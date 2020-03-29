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

#include <fstream>

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
    iElfParser = new ElfParser(iOpts->iElfinput);
    iElfParser->GetElfFileLayout();
    SymbolProcessor processor(iElfParser, iOpts);
    iSymbols = processor.GetExports();
}

void ArtifactBuilder::MakeDSO()
{
    if(iOpts->iDso.empty())
        return;
    DSOFile* dso = new DSOFile();
    dso->WriteDSOFile(iOpts->iDso, iOpts->iLinkas, iSymbols);
    delete dso;
}

void ArtifactBuilder::MakeDef()
{
    if(iOpts->iDefoutput.empty())
        return;
    DefFile deffile;
	deffile.WriteDefFile(iOpts->iDefoutput.c_str(), iSymbols);
}

void ArtifactBuilder::MakeE32()
{
    if(!iOpts->iOutput.empty())
        return;
    BuildE32Image(iOpts, iElfParser);
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

//Function symbol for any ECOM plugin load
const char aniExportName[] = "_Z15CreateCAnimDllLv,1;";
const char ecomExportName[] = "_Z24ImplementationGroupProxyRi,1;";
const char fsyExportName[] = "CreateFileSystem,1;";
const char lddExportName[] = "_Z19CreateLogicalDevicev,1;";
const char pddExportName[] = "_Z20CreatePhysicalDevicev,1;";
const char pdlExportName[] = "_Z17NewPrinterDeviceLv,1;";
const char textNotifierExportName[] = "_Z13NotifierArrayv,1;";
const char varExportName[] = "_Z17VariantInitialisev,1;";
const char var2ExportName[] = "VariantInitialise,1;";

/** \brief Verifies and correct wrong input options
 * This function correct multiple conflict opions
 * like --datapaging with different params,
 * also fix wrong uid1 for exe and dll
 *
 */
void ValidateOptions(Args* arg)
{
    FixHeaderName(arg);

    if(!arg->iHeader.empty() && !arg->iElfinput.empty())
        return;

    if(arg->iLinkas.empty())
    {
        std::string linkas;
        if(!arg->iOutput.empty())
        {
            linkas = arg->iOutput;
            linkas.insert(linkas.find_last_of("."), "{000a0000}");
        }
        else if(!arg->iDefoutput.empty())
        {
            linkas = arg->iDefoutput;
            linkas.insert(linkas.find_last_of("."), "{000a0000}");
            linkas.erase(linkas.find_last_of("."));
            linkas += ".dll";
        }
        else if(!arg->iDso.empty())
        {
            linkas = arg->iDso;
            linkas.insert(linkas.find_last_of("."), "{000a0000}");
            linkas.erase(linkas.find_last_of("."));
            linkas += ".dll";
        }
        arg->iLinkas = linkas;
    }

    bool hasDefinput = !arg->iDefinput.empty();
    bool noDefOut = arg->iDefoutput.empty();
    bool noElfinput = arg->iElfinput.empty();
    bool noE32Image = arg->iOutput.empty();
    TargetType iTargetType = arg->iTargettype;

	if(iTargetType == TargetType::EInvalidTargetType || iTargetType == TargetType::ETargetTypeNotSet)
	{
	    ReportWarning(ErrorCodes::NOREQUIREDOPTION, "--targettype");
        if(hasDefinput)
            arg->iTargettype = TargetType::EImportLib;
// handle special Custom target
        else if(!noDefOut || !arg->iDso.empty())
            arg->iTargettype = TargetType::EDll;
        else
            arg->iTargettype = TargetType::EExe;
	}
	if((iTargetType == TargetType::EPlugin) && arg->iSysdef.empty())

    if(arg->iSysdef.empty())
    {
        if((iTargetType == TargetType::EPlugin) || (iTargetType == TargetType::EPlugin3))
            arg->iSysdef = ecomExportName;
        else if(iTargetType == TargetType::EAni)
            arg->iSysdef = aniExportName;
        else if(iTargetType == TargetType::EFsy)
            arg->iSysdef = fsyExportName;        else if(iTargetType == TargetType::ELdd)
            arg->iSysdef = lddExportName;
        else if(iTargetType == TargetType::EPdd)
            arg->iSysdef = pddExportName;
        else if(iTargetType == TargetType::EPdl)
            arg->iSysdef = pdlExportName;
        else if(iTargetType == TargetType::ETextNotifier2)
            arg->iSysdef = textNotifierExportName;
        else if(iTargetType == TargetType::EVar)
            arg->iSysdef = varExportName;
        else if(iTargetType == TargetType::EVar2)
            arg->iSysdef = var2ExportName;
    }

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
        if(!arg->iDso.empty())
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--dso");
		break;
    case TargetType::EStdDll:
        if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
		if(noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");

		if(UID1 != KDynamicLibraryUidValue) //< guard against wrong uids
		{
		    ReportLog("********************\n");
		    ReportLog("Wrong UID1\n");
		    ReportLog("Set uid1 to KDynamicLibraryUidValue\n");
		    ReportLog("********************\n");
		}
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
		{
		    ReportLog("********************\n");
		    ReportLog("Wrong UID1\n");
		    ReportLog("Set uid1 to KDynamicLibraryUidValue\n");
		    ReportLog("********************\n");
		}
        arg->iUid1 = KDynamicLibraryUidValue;
		if(!UID2)
		{
		    ReportLog("********************\n");
		    ReportLog("missed value for UID2\n");		    ReportLog("********************\n");
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
		{
		    ReportLog("********************\n");
		    ReportLog("Wrong UID1\n");
		    ReportLog("Set uid1 to KExecutableImageUidValue\n");
		    ReportLog("********************\n");
		}
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
		if(noElfinput)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--elfinput");
		if(noE32Image)
            ReportError(ErrorCodes::NOREQUIREDOPTION, "--output");
		if(UID1 != KDynamicLibraryUidValue)
		{
		    ReportLog("********************\n");
		    ReportLog("Wrong UID1\n");
		    ReportLog("Set uid1 to KDynamicLibraryUidValue\n");
		    ReportLog("********************\n");
		}
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
		{
		    ReportLog("********************\n");
		    ReportLog("Wrong UID1\n");
		    ReportLog("Set uid1 to KExecutableImageUidValue\n");
		    ReportLog("********************\n");
        }
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
        {
		    ReportLog("********************\n");
		    ReportLog("Wrong UID1\n");
		    ReportLog("Set uid1 to KExecutableImageUidValue\n");		    ReportLog("********************\n");
        }
        arg->iUid1 = KExecutableImageUidValue;
        arg->iUid2 = KSTDTargetUid2Value;

		if(!UID3) ReportLog("Missed --uid3 option!\n");
		break;
	default:
		break;
	}
}

