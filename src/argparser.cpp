// Copyright (c) 2019-2020 Strizhniou Fiodar
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
#include <string.h>

#include "getopt.h"
#include "common.hpp"
#include "argparser.h"
#include "e32common.h"
#include "elf2e32_opt.hpp"
#include "cmdlineprocessor.h"
#include "elf2e32_version.hpp"

using std::string;

static struct option long_opts[] =
{
    {"uid1",  required_argument, nullptr, OptionsType::EUID1},
    {"uid2",  required_argument, nullptr, OptionsType::EUID2},
    {"uid3",  required_argument, nullptr, OptionsType::EUID3},
    {"sid",   required_argument, nullptr, OptionsType::ESID},
    {"vid",   required_argument, nullptr, OptionsType::EVID},
    {"heap",  required_argument, nullptr, OptionsType::EHEAP},
    {"stack", required_argument, nullptr, OptionsType::ESTACK},
    // for E32ImageHeader::iFlags
    {"fixedaddress",     no_argument, nullptr, OptionsType::EFIXEDADDRESS},
    {"callentry",        no_argument, nullptr, OptionsType::ECALLENTRY},
    {"fpu",        required_argument, nullptr, OptionsType::EFPU},
    {"codepaging", required_argument, nullptr, OptionsType::ECODEPAGING},
    {"datapaging", required_argument, nullptr, OptionsType::EDATAPAGING},
    {"paged",            no_argument, nullptr, OptionsType::EPAGED},
    {"defaultpaged",     no_argument, nullptr, OptionsType::EDEFAULTPAGED},
    {"unpaged",          no_argument, nullptr, OptionsType::EUNPAGED},
    {"debuggable",       no_argument, nullptr, OptionsType::EDEBUGGABLE},
    {"smpsafe",          no_argument, nullptr, OptionsType::ESMPSAFE},
    // for image generation
    {"targettype",   required_argument, nullptr, OptionsType::ETARGETTYPE},
    {"linkas",       required_argument, nullptr, OptionsType::ELINKAS},
    {"uncompressed",       no_argument, nullptr, OptionsType::EUNCOMPRESSED},
    {"compressionmethod", required_argument, nullptr, OptionsType::ECOMPRESSIONMETHOD},
    {"unfrozen",           no_argument, nullptr, OptionsType::EUNFROZEN},
    {"ignorenoncallable",  no_argument, nullptr, OptionsType::EIGNORENONCALLABLE},
    {"capability",   required_argument, nullptr, OptionsType::ECAPABILITY},
    {"sysdef",       required_argument, nullptr, OptionsType::ESYSDEF},
    {"dlldata",            no_argument, nullptr, OptionsType::EDLLDATA},
    {"priority",     required_argument, nullptr, OptionsType::EPRIORITY},
    {"excludeunwantedexports",   no_argument, nullptr, OptionsType::EEXCLUDEUNWANTEDEXPORTS},
    {"customdlltarget",    no_argument, nullptr, OptionsType::ECUSTOMDLLTARGET},
    {"namedlookup",        no_argument, nullptr, OptionsType::ENAMEDLOOKUP},
    // input files
    {"definput",  required_argument, nullptr, OptionsType::EDEFINPUT},
    {"defoutput", required_argument, nullptr, OptionsType::EDEFOUTPUT},
    {"elfinput",  required_argument, nullptr, OptionsType::EELFINPUT},
    {"dsodump",         no_argument, nullptr, OptionsType::EDSODUMP},
    {"output",    required_argument, nullptr, OptionsType::EOUTPUT},
    {"dso",       required_argument, nullptr, OptionsType::EDSO},
    {"libpath",   required_argument, nullptr, OptionsType::ELIBPATH},
    {"e32input",  required_argument, nullptr, OptionsType::EE32INPUT},
    {"header",    optional_argument, nullptr, OptionsType::EHEADER},
    // info for E32 image
    {"dump",      required_argument, nullptr, OptionsType::EDUMP},
    // common options
    {"log",             required_argument, nullptr, OptionsType::ELOG},
    {"version",         required_argument, nullptr, OptionsType::EVERSION},
    {"man",                   no_argument, nullptr, OptionsType::EMAN},
    {"man-edit",              no_argument, nullptr, OptionsType::EMANEDIT},
    {"man-build",             no_argument, nullptr, OptionsType::EMANBUILD},
    {"man-build-artifacts",   no_argument, nullptr, OptionsType::EMANARTIFACTS},
    {"help",                  no_argument, nullptr, OptionsType::EHELP},
    // dev options
    {"time",            required_argument, nullptr, OptionsType::TIME},
    // ignored options
    {"messagefile",     required_argument, nullptr, OptionsType::EMESSAGEFILE},
    {"dumpmessagefile", required_argument, nullptr, OptionsType::EDUMPMESSAGEFILE},
    {nullptr,0,nullptr,0}
};

ArgParser::ArgParser(int argc, char** argv): iArgc(argc), iArgv(argv)
{
    opterr = 0;
}

ArgParser::~ArgParser()
{
    //dtor
}

void ArgName(const char* name); // long_opts[optIdx].name
void ArgInfo(const char* name, const char* opt = nullptr); // long_opts[optIdx].name, optarg
void Help();
uint32_t Str2Hex(const char* s);

//const string man =
//"Эта программа предназначена для создания E32Image и сопутствующих файлов"
//" (--man-build). Помимо этого программа может изменять некоторые свойства.\n"
const string man =
"This program is designed to create E32Image and related files (--man-build)."
" In addition, the program can change some properties of E32Image (--man-edit).\n"
;

//const string manBuild =
//"Создание E32Image, def, dso and import header(далее - артефакты) представлена 2 режимами:"
//"\t* получение списка экспортируемых символов из dso и создание def, import header(--man-build-dsodump)"
//"\t* создание артефактов из def, sysdef, elf(--man-build-artifacts).\n";
const string manBuild =
"Creating E32Image, def, dso and import header (hereinafter referred "
"to as artifacts) is represented by 2 modes:\n"
"\t* receiving a list of exported symbols from dso and creating def, import header (--man-build-dsodump)\n"
"\t* creating artifacts from def, sysdef, elf (--man-build-artifacts).\n"
;

const string manDsoDump =
" Options --elfinput and --defoutput are used together to enable the creation of artifacts from .dso mode,"
" --elfinput is used to load data, assembly artifacts are specified via --header and --defoutput\n"
;
//const string manArtifacts =
//"Создание E32Image и прочих артефактов управляется соответствующими флагами."
//" Для создания некоторых артефактов требуются задать определенные парараметры командной строки:\n"
//"\t* --output и --elfinput требуются для создания E32Image, для dll и exedll могут заданы"
//" следующие параметры --sysdef, --definput.\n"
//"\t* требуется хотя бы один из следующих параметров: --elfinput, --sysdef, --definput,"
//" для создания любого из них: .def, .dso, import header.\n";
const string manArtifacts =
"The creation of E32Image and other artifacts is controlled by the corresponding flags."
" To create some artifacts, you need to set certain command line parameters:\n"
"\t* --output and --elfinput are required to create an E32Image, for dll and exedll the"
" following parameters can be specified --sysdef, --definput.\n"
"\t* At least one of the following parameters is required: --elfinput, --sysdef,"
" --definput, to create any of them: .def, .dso, import header.\n"
;
//const string manEdit =
//"Для изменения E32Image требуется два флага: --e32input и --output."
//" Можно изменять следующие свойства файла: алгоритм сжатия, UIDs, Heap committed and"
//" reserved size, specify the process priority for your executable EXE,"
//" задать новые capability при этом время сохранения файла станет временем создания файла, "
//"а версией утидиты создавшей файл станет текущая версия elf2e32.\n";
const string manEdit =
"Two flags are required to modify E32Image: --e32input and --output."
" You can change the following file properties: compression algorithm, UIDs,"
" Heap committed and reserved size, specify the process priority for your"
" executable EXE, set new capabilities while the file save time will become"
" the file creation time, and the current version of elf2e32 will become the version of the"
" utility that created the file.\n"
;

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
                arg->iUid1 = Str2Hex(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EUID2:
                arg->iUid2 = Str2Hex(optarg);
                VarningForDeprecatedUID(arg->iUid2);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EUID3:
                arg->iUid3 = Str2Hex(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::ESID:
                arg->iSid = Str2Hex(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EVID:
                arg->iVid = Str2Hex(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EHEAP:
            {
                arg->iHeapMin = Str2Hex(optarg);
                string t(optarg);
                arg->iHeapMax = Str2Hex(t.substr( t.find_first_of(",.;") + 1 ).c_str());
                ArgInfo(optname, optarg);
                break;
            }
            case OptionsType::ESTACK:
                arg->iStack = Str2Hex(optarg);
                ArgInfo(optname, optarg);
                break;
        // for E32ImageHeader::iFlags
            case OptionsType::EFIXEDADDRESS:
                arg->iFixedaddress = true;
                ArgInfo(optname);
                break;
            case OptionsType::EFPU:
                arg->iFpu = GetFpuType(optarg);
                ArgInfo(optname ,optarg);
                break;
            case OptionsType::ECODEPAGING:
                arg->iCodePaging = GetPaging(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EDATAPAGING:
                arg->iDataPaging = GetPaging(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::EPAGED:
                arg->iCodePaging = Paging::PAGED;
                ArgInfo(optname);
                break;
            case OptionsType::EUNPAGED:
                arg->iCodePaging = Paging::UNPAGED;
                ArgInfo(optname);
                break;
            case OptionsType::EDEFAULTPAGED:
                arg->iCodePaging = Paging::DEFAULT;
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
                arg->iTargettype = GetTarget(optarg);
                ArgInfo(optname, optarg);
                break;
            case OptionsType::ELINKAS:
                arg->iLinkas = optarg;
                ArgInfo(optname ,optarg);
                break;
            case OptionsType::EUNCOMPRESSED:
                arg->iCompressionMethod = KFormatNotCompressed;
                ArgInfo(optname);
                break;
            case OptionsType::ECOMPRESSIONMETHOD:
            {
                if(!strcasecmp(optarg, "none"))
                    arg->iCompressionMethod = KFormatNotCompressed;
                else if(!strcasecmp(optarg, "inflate"))
                    arg->iCompressionMethod = KUidCompressionDeflate;
                else if(!strcasecmp(optarg, "bytepair"))
                    arg->iCompressionMethod = KUidCompressionBytePair;
                else
                    arg->iCompressionMethod = KUidCompressionDeflate;
                ArgInfo(optname ,optarg);
                break;
            }
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
            case OptionsType::EDSODUMP:
                arg->iDSODump = true;
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
            case OptionsType::EHEADER:
                if(optarg)
                    arg->iHeader = optarg;
                else arg->iHeader = "not_set";
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
                ArgInfo(optname ,optarg);
                break;
            case OptionsType::EMAN:
                ReportLog(man);
                ArgInfo(optname);
                break;
            case OptionsType::EMANEDIT:
                ReportLog(manEdit);
                ArgInfo(optname);
                break;
            case OptionsType::EMANBUILD:
                ReportLog(manBuild);
                ArgInfo(optname);
                break;
            case OptionsType::EMANDSODUMP:
                ReportLog(manDsoDump);
                ArgInfo(optname);
                break;
        // dev options
            case OptionsType::TIME: // --time=hi,low
            {
                arg->iTime[0] = Str2Hex(optarg); // iTimeHi
                string t(optarg);
                arg->iTime[1] = Str2Hex(t.substr( t.find_first_of(",") + 1 ).c_str());
            }
                ArgInfo(optname ,optarg);
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
            case OptionsType::ECALLENTRY:
            case OptionsType::EMESSAGEFILE: // fallthru
            case OptionsType::EDUMPMESSAGEFILE: // fallthru
                break;
            default:
                Help();
                return false;
        }
    };
    return true;
}

const string ScreenOptions =
"        --definput=Input DEF File\n"
"        --defoutput=Output DEF\n"
"        --elfinput=Input ELF File\n"
"           --dsodump: Get symbols from DSO\n"
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
"        --unfrozen: Dot-def file preserve stable library ABI. Breaks ABI if used.\n"
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
"        --priority=Specify the process priority for your executable EXE\n"
"        --version=Module Version\n"
"        --callentry=Call Entry Point(ignored)\n"
"        --fpu=FPU type [softvfp|vfpv2|vfpv3|vfpv3D16]\n"
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
"        --header: Generate C++ header file for dynamic linking.\n"
"        --man: Describe advanced usage new features.\n"
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

void ArgName(const char *name) // long_opts[*optIdx].name
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
    ArgName(name);
    if(name && opt)
    {
        ReportLog("=");
        ReportLog(opt);
    }
    ReportLog("\n");
}

uint32_t Str2Hex(const char* s)
{
    if((s[0] == '0') && ((s[1] == 'x')||(s[1] == 'X')))
        return std::stoi(s, nullptr, 16);
    return std::stoi(s);
}
