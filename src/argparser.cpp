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
// Parse cmdline arguments for the elf2e32 tool
//
//

#include <string>
#include "getopt.h"
#include "common.hpp"
#include "argparser.h"
#include "elf2e32_opt.hpp"
#include "elf2e32_version.hpp"

using std::string;

uint32_t SetToolVersion(const char* str);

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

ArgParser::ArgParser(int argc, char** argv): iArgc(argc), iArgv(argv)
{
    opterr = 0;
}

ArgParser::~ArgParser()
{
    //dtor
}

void ArgInfo(const char *name); // long_opts[optIdx].name
void ArgInfo(const char *name, const char* opt); // long_opts[optIdx].name, optarg
void Help();

bool ArgParser::Parse(Args* arg) const
{
    if(iArgc == 1)
    {
        Help();
        return false;
    }

    int rez, optIdx;
    const char* optname = nullptr;
    while( (rez = getopt_long(iArgc, iArgv, nullptr, long_opts, &optIdx)) != -1)
    {
        if((rez != ':') && (rez != '?'))
            optname = long_opts[optIdx].name;

        switch(rez)
        {
            case OptionsType::EUID1:
                arg->iUid1 = std::stoi(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EUID2:
                arg->iUid2 = std::stoi(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EUID3:
                arg->iUid3 = std::stoi(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::ESID:
                arg->iSid = std::stoi(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EVID:
                arg->iVid = std::stoi(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EHEAP:
            {
                arg->iHeapMin = std::stoi(optarg);
                string t(optarg);
                arg->iHeapMax = std::stoi(t.substr( t.find_first_of(",.;") + 1 ));
                ArgInfo(optname, optarg);
                break;
            }
            case OptionsType::ESTACK:
                arg->iStack = std::stoi(optarg);
                ArgInfo(optname, optarg);
                break;
        // for E32ImageHeader::iFlags
            case OptionsType::EFIXEDADDRESS:
                arg->iFixedaddress = true;
                ArgInfo(optname);
                break;
            case OptionsType::ECALLENTRY:
                arg->iCallentry = true;
                ArgInfo(optname);
                break;
            case OptionsType::EFPU:
                arg->iFpu = optarg;
                ArgInfo(optname ,optarg);
                break;
            case OptionsType::ECODEPAGING:
                arg->iCodepaging = true;
                ArgInfo(optname);
                break;
            case OptionsType::EDATAPAGING:
                arg->iDatapaging = true;
                ArgInfo(optname);
                break;
            case OptionsType::EPAGED:
                arg->iPaged = true;
                ArgInfo(optname);
                break;
            case OptionsType::EUNPAGED:
                arg->iUnpaged = true;
                ArgInfo(optname);
                break;
            case OptionsType::EDEFAULTPAGED:
                arg->iDefaultpaged = true;
                ArgInfo(optname);
                break;
            case OptionsType::EDEBUGGABLE:
                arg->iDebuggable = true;
                ArgInfo(optname);
                break;
            case OptionsType::ESMPSAFE:
                arg->iSmpsafe = true;
                ArgInfo(optname);
                break;
        // for image generation
            case OptionsType::ETARGETTYPE:
                arg->iTargettype = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::ELINKAS:
                arg->iLinkas = optarg;
                ArgInfo(optname ,optarg);
                break;
            case OptionsType::EUNCOMPRESSED:
                arg->iCompressionMethod = 0; //KFormatNotCompressed
                ArgInfo(optname);
                break;
            case OptionsType::ECOMPRESSIONMETHOD:
                arg->iCompressionMethod = std::stoi(optarg);;
                ArgInfo(optname ,optarg);
                break;
            case OptionsType::EUNFROZEN:
                arg->iUnfrozen = true;
                ArgInfo(optname);
                break;
            case OptionsType::EIGNORENONCALLABLE:
                arg->iIgnorenoncallable = true;
                ArgInfo(optname);
                break;
            case OptionsType::ECAPABILITY:
                arg->iCapability = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::ESYSDEF:
                arg->iSysdef = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EDLLDATA:
                arg->iDlldata = true;
                ArgInfo(optname);
                break;
            case OptionsType::EPRIORITY:
                arg->iPriority = ProcessPriority(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EEXCLUDEUNWANTEDEXPORTS:
                arg->iExcludeunwantedexports = true;
                ArgInfo(optname);
                break;
            case OptionsType::ECUSTOMDLLTARGET:
                arg->iCustomdlltarget = true;
                ArgInfo(optname);
                break;
            case OptionsType::ENAMEDLOOKUP:
                arg->iNamedlookup = true;
                ArgInfo(optname);
                break;
        // input files
            case OptionsType::EDEFINPUT:
                arg->iDefinput = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EDEFOUTPUT:
                arg->iDefoutput = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EELFINPUT:
                arg->iElfinput = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EOUTPUT:
                arg->iOutput = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EDSO:
                arg->iDso = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::ELIBPATH:
                arg->iLibpath = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EE32INPUT:
                arg->iE32input = optarg;
                ArgInfo(optname, optarg);
                break;
        // info for E32 image
            case OptionsType::EDUMP:
                arg->iDump = optarg;
                ArgInfo(optname ,optarg);
                break;
        // common options
            case OptionsType::ELOG:
                arg->iLog = optarg;
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EVERSION:

                arg->iVersion = SetToolVersion(optarg);
                ArgInfo(optname);
                break;

            case ':':
                //optind always point to next argv instead current                ReportError(MISSEDARGUMENT, iArgv[optind-1], Help);
                return false;
            case '?':
                //optind always point to next argv instead current
                ReportError(UNKNOWNOPTION, iArgv[optind-1], Help);
                return false;
            case OptionsType::EHELP:
                Help();
                return false;
        //silently ignored options
            case OptionsType::EMESSAGEFILE: // fallthru
            case OptionsType::EDUMPMESSAGEFILE:
                break;
            default:
                Help();
                return false;
        }
    };
    return true;
}

uint32_t SetToolVersion(const char* str)
{
	ReportLog("!!! check input param for SetToolVersion():\n");
	ReportLog(str);
	ReportLog("!!!********!!!\n");

	uint32_t hi, lo;
	hi = std::stoi(str);
    string t(str);
    lo = std::stoi(t.substr( t.find_first_of(".,;") + 1 ));
    return ((hi & 0xFFFF) << 16) | (lo & 0xFFFF);
}

const string ScreenOptions =
"        --definput=Input DEF File\n"
"        --defoutput=Output DEF\n"
"        --elfinput=Input ELF File\n"
"        --output=Output E32 Image\n"
"        --dso=Output import DSO File\n"
"        --targettype=Target Type\n"
"        --linkas=name\n"
"        --uid1=UID 1\n"
"        --uid2=UID 2\n"
"        --uid3=UID 3\n"
"        --sid=Secure ID\n"
"        --vid=Vendor ID\n"
"        --fixedaddress: Has fixed address\n"
"        --uncompressed: Don't compress output e32image\n"
"        --compressionmethod=Input compression method [none|inflate|bytepair]\n"
"                none     no compress the image.\n"
"                inflate  compress image with Inflate algorithm.\n"
"                bytepair compress image with BytePair Pak algorithm.\n"
"        --heap=Heap committed and reserved size in bytes(.EXEs only), comma separated.\n"
"        --stack=Stack size in bytes(.EXEs only)\n"
"        --unfrozen: Don't treat input dot-def file as frozen\n"
"        --ignorenoncallable: Generate exports for functions only\n"
"        --capability=Capability option\n"
"        --libpath=A semi-colon separated search path list to locate import DSOs\n"
"        --sysdef=A semi-colon separated predefined Symbols to be exported and the ordinal number\n"
"        --log=Redirect tool messages to file\n"
"        --messagefile=Input Message File(ignored)\n"
"        --dumpmessagefile=Output Message File(ignored)\n"
"        --dlldata: Allow writable static data in DLL\n"
"        --dump=Input dump options [hscdeit] --e32input <filename>\n"
"        Flags for dump:\n"
"                h Header\n"
"                s Security info\n"
"                c Code section\n"
"                d Data section\n"
"                e Export info\n"
"                i Import table\n"
"                t Symbol Info\n"
"        --e32input=Input E32 Image file name\n"
"        --priority=Input Priority\n"
"        --version=Module Version\n"
"        --callentry=Call Entry Point\n"
"        --fpu=FPU type [softvfp|vfpv2]\n"
"        --codepaging=Code Paging Strategy [paged|unpaged|default]\n"
"        --datapaging=Data Paging Strategy [paged|unpaged|default]\n"
"        --paged: This option is deprecated. Use --codepaging=paged instead.\n"
"        --unpaged: This option is deprecated. Use --codepaging=unpaged instead.\n"
"        --defaultpaged: This option is deprecated. Use --codepaging=default instead.\n"
"        --excludeunwantedexports: Ignore builded exports not marked as EXPORT_C\n"
"        --customdlltarget: Custom Dll Target\n"
"        --namedlookup: Enable named lookup of symbols\n"
"        --debuggable: Debuggable by run-mode debug subsystem\n"
"        --smpsafe: SMP Safe\n"
"        --help: This command.\n"
;

void Help()
{
    ToolVersion v;
    ReportLog("\nSymbian Post Linker, Elf2E32 v %d.%d (Build %d)\n",
              (int16_t)v.iMajor, (int16_t)v.iMinor, (int16_t)v.iBuild);
    ReportLog("Usage:  elf2e32 [options] [filename]\n\n");
    ReportLog("Options:\n");
    ReportLog(ScreenOptions);
}

void ArgInfo(const char *name) // long_opts[*optIdx].name
{
    #if !_DEBUG
    return;
    #endif
    if(!name)
    {
        ReportLog("Option not send!!!\n");
        return;
    }
    ReportLog("Got option: --");
    ReportLog(name);
}

void ArgInfo(const char *name, const char* opt) // long_opts[*optIdx].name, optarg
{
    #if !_DEBUG
    return;
    #endif
    ArgInfo(name);
    if(name && opt)
    {
        ReportLog("=");
        ReportLog(opt);
    }
    ReportLog("\n");
}

