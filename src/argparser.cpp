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
// Parse cmdline arguments for the elf2e32 tool
//
//

#include <string>
#include <sstream>
#include <string.h>
#include <strings.h>

#include "common.hpp"
#include "getopt.hpp"
#include "argparser.h"
#include "e32common.h"
#include "getopt_opts.h"
#include "elf2e32_opt.hpp"
#include "cmdlineprocessor.h"
#include "elf2e32_version.hpp"

using std::string;

enum VerbosePrint
{
    DISABLE_LONG_PRINT = -1,
    NONE = 0,
    ENABLE_ALL = 1,
};

static int VerboseOutput = VerbosePrint::NONE;

bool VerboseOut() {return VerboseOutput;}
bool DisableLongVerbosePrint() {return VerboseOutput == VerbosePrint::DISABLE_LONG_PRINT;}

ArgParser::ArgParser(int argc, char** argv)
{
    iArgc = argc;
    iArgv.insert(iArgv.begin(), argv, argv + argc);
}

ArgParser::ArgParser(std::vector<std::string> argv, const struct Opts* opt)
{
    iArgc = argv.size();
    iArgv = argv;
    iTests = opt;
}

ArgParser::~ArgParser()
{
    //dtor
}

void Help();

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

    Opts op;
    for(int i = 1; i < iArgc; i++) // skip iArgv[0]
    {
        op = getopt(iArgv[i]);

        switch(op.val)
        {
            case OptionsType::EUID1:
                arg->iUid1 = strtoul(op.arg.c_str(), nullptr, 16);
                op.binary_arg1 = arg->iUid1;
                break;
            case OptionsType::EUID2:
                arg->iUid2 = strtoul(op.arg.c_str(), nullptr, 16);
                op.binary_arg1 = arg->iUid2;
                VarningForDeprecatedUID(arg->iUid2);
                break;
            case OptionsType::EUID3:
                arg->iUid3 = strtoul(op.arg.c_str(), nullptr, 16);
                op.binary_arg1 = arg->iUid3;
                arg->iLinkasUid = op.arg;
                break;
            case OptionsType::ESID:
                arg->iSid = strtoul(op.arg.c_str(), nullptr, 16);
                op.binary_arg1 = arg->iSid;
                break;
            case OptionsType::EVID:
                arg->iVid = strtoul(op.arg.c_str(), nullptr, 16);
                op.binary_arg1 = arg->iVid;
                break;
            case OptionsType::EHEAP:            {
                arg->iHeapMin = strtoul(op.arg.c_str(), nullptr, 16);
                string t(op.arg);
                arg->iHeapMax = strtoul(t.substr( t.find_first_of(",.;") + 1 ).c_str(), nullptr, 16);
                op.binary_arg1 = arg->iHeapMin;
                op.binary_arg2 = arg->iHeapMax;
                break;
            }
            case OptionsType::ESTACK:
                arg->iStack = strtoul(op.arg.c_str(), nullptr, 16);
                op.binary_arg1 = arg->iStack;
                break;
        // for E32ImageHeader::iFlags
            case OptionsType::EFIXEDADDRESS:
                arg->iFixedaddress = true;
                op.binary_arg1 = true;
                break;
            case OptionsType::EFPU:
                arg->iFpu = GetFpuType(op.arg);
                op.binary_arg1 = arg->iFpu;
                break;
            case OptionsType::ECODEPAGING:
                arg->iCodePaging = GetPaging(op.arg);
                op.binary_arg1 = arg->iCodePaging;
                break;
            case OptionsType::EDATAPAGING:
                arg->iDataPaging = GetPaging(op.arg);
                op.binary_arg1 = arg->iDataPaging;
                break;
            case OptionsType::EPAGED:
                arg->iCodePaging = Paging::PAGED;
                op.binary_arg1 = Paging::PAGED;
                break;
            case OptionsType::EUNPAGED:
                arg->iCodePaging = Paging::UNPAGED;
                op.binary_arg1 = Paging::UNPAGED;
                break;
            case OptionsType::EDEFAULTPAGED:
                arg->iCodePaging = Paging::DEFAULT;
                op.binary_arg1 = Paging::DEFAULT;
                break;
            case OptionsType::EDEBUGGABLE:
                arg->iDebuggable = true;
                op.binary_arg1 = true;
                break;
            case OptionsType::ESMPSAFE:
                arg->iSmpsafe = true;
                op.binary_arg1 = true;
                break;
        // for image generation
            case OptionsType::ETARGETTYPE:
                arg->iTargettype = GetTarget(op.arg);
                op.binary_arg1 = arg->iTargettype;
                break;
            case OptionsType::ELINKAS:
                arg->iLinkas = op.arg;
                break;
            case OptionsType::EUNCOMPRESSED:
                arg->iCompressionMethod = KFormatNotCompressed;
                op.binary_arg1 = KFormatNotCompressed;
                break;
            case OptionsType::ECOMPRESSIONMETHOD:
            {
                if(!strcasecmp(op.arg.c_str(), "none"))
                    arg->iCompressionMethod = KFormatNotCompressed;
                else if(!strcasecmp(op.arg.c_str(), "inflate"))
                    arg->iCompressionMethod = KUidCompressionDeflate;
                else if(!strcasecmp(op.arg.c_str(), "bytepair"))
                    arg->iCompressionMethod = KUidCompressionBytePair;
                else
                    arg->iCompressionMethod = KUidCompressionDeflate;

                op.binary_arg1 = arg->iCompressionMethod;
                break;
            }
            case OptionsType::EUNFROZEN:
                arg->iUnfrozen = true;
                op.binary_arg1 = true;
                break;
            case OptionsType::EIGNORENONCALLABLE:
                arg->iIgnorenoncallable = true;
                op.binary_arg1 = true;
                break;
            case OptionsType::ECAPABILITY:
                arg->iCapability = op.arg;
                break;
            case OptionsType::ESYSDEF:
                arg->iSysdef = op.arg;
                break;
            case OptionsType::EDLLDATA:
                arg->iNoDlldata = false;
                op.binary_arg1 = false;
                break;
            case OptionsType::EPRIORITY:
                arg->iPriority = ProcessPriority(op.arg);
                op.binary_arg1 = arg->iPriority;
                break;
            case OptionsType::EEXCLUDEUNWANTEDEXPORTS:
                arg->iExcludeunwantedexports = true;
                op.binary_arg1 = true;
                break;
            case OptionsType::ECUSTOMDLLTARGET:
                arg->iCustomdlltarget = true;
                op.binary_arg1 = true;
                break;
            case OptionsType::ENAMEDLOOKUP:
                arg->iNamedlookup = true;
                op.binary_arg1 = true;
                break;
        // input files
            case OptionsType::EDEFINPUT:
                arg->iDefinput = op.arg;
                break;
            case OptionsType::EDEFOUTPUT:
                arg->iDefoutput = op.arg;
                break;
            case OptionsType::EELFINPUT:
                arg->iElfinput = op.arg;
                break;
            case OptionsType::EOUTPUT:
                arg->iOutput = op.arg;
                break;
            case OptionsType::EDSO:
                arg->iDso = op.arg;
                break;
            case OptionsType::ELIBPATH:
                arg->iLibpath = op.arg;
                break;
            case OptionsType::EE32INPUT:
                arg->iE32input = op.arg;
                break;
            case OptionsType::EHEADER:
                if(op.arg.empty())
                    arg->iHeader = "not_set";
                else arg->iHeader = op.arg;
                break;
        // info for E32 image
            case OptionsType::EDUMP:
                arg->iDump = op.arg;
                break;
        // common options
            case OptionsType::ELOG:
                arg->iLog = op.arg;
                break;
            case OptionsType::EVERSION:
                arg->iVersion = SetToolVersion(op.arg);
                op.binary_arg1 = arg->iVersion;
                break;
            case OptionsType::EMAN:
                ReportLog(man);
                break;
            case OptionsType::EMANEDIT:
                ReportLog(manEdit);
                break;
            case OptionsType::EMANBUILD:
                ReportLog(manBuild);
                break;
            case OptionsType::EMANDSODUMP:
                ReportLog(manDsoDump);
                break;
            case OptionsType::EMANARTIFACTS:
                ReportLog(manArtifacts);
                break;
        // dev options
            case OptionsType::TIME: // --time=hi,low
            {
                arg->iTime[0] = strtoul(op.arg.c_str(), nullptr, 16); // iTimeHi
                string t(op.arg);
                arg->iTime[1] = strtoul(t.substr( t.find_first_of(",") + 1 ).c_str(), nullptr, 16);
                op.binary_arg1 = arg->iTime[0];
                op.binary_arg2 = arg->iTime[1];
                break;
            }
            case OptionsType::FILECRC:
                if(op.arg.empty())
                {
                    arg->iFileCrc = DefaultCrcFiles();
                    op.arg = DefaultOptionalArg;
                }
                else
                    arg->iFileCrc = CrcFiles(op.arg);
                break;
            case OptionsType::VERBOSE:
                if(!op.arg.empty())
                {
                    VerboseOutput = strtol(op.arg.c_str(), nullptr, 10);
                }
                else
                    VerboseOutput = VerbosePrint::ENABLE_ALL;
                op.binary_arg1 = VerboseOutput;
                break;
            case OptionsType::FORCEE32BUILD:
                arg->iForceE32Build = true;
                op.binary_arg1 = true;
                break;
            case OptionsType::EMISSEDARG:
                ReportError(MISSEDARGUMENT, iArgv[i], Help);
                return false;
            case OptionsType::ENOTRECOGNIZEDARG:
                ReportError(UNKNOWNOPTION, iArgv[i], Help);
                return false;
            case OptionsType::EHELP:
                Help();
                return false;
            case OptionsType::ECALLENTRY:
                arg->iCallentry = true;
                op.binary_arg1 = true;
                break;
        //silently ignored options
            case OptionsType::EMESSAGEFILE: // fallthru
            case OptionsType::EDUMPMESSAGEFILE: // fallthru
                break;
        //ignored options
            case OptionsType::EASM:
                ReportLog("Unsupported option for BELLE SDK --" + op.name + "/n");
                op.binary_arg1 = true;
                break;
            case OptionsType::EE32TRAN:
                ReportLog("Unsupported option for BELLE SDK --" + op.name + "=" + op.arg + "/n");
                op.binary_arg1 = true;
                break;
            case OptionsType::EEXPORTAUTOUPDATE:
                ReportLog("Unsupported option for BELLE SDK --" + op.name + "/n");
                op.binary_arg1 = true;
                break;
            default:
                Help();
                return false;
        }
        ArgInfo(op);
    };
#if ELF2E32_PRINT_INPUT_ARGS
    if(1)
#else
    if(VerboseOutput && !DisableLongVerbosePrint())
#endif // ELF2E32_PRINT_INPUT_ARGS
    {
        printf("Args to parse: \n");
        for(int i = 0; i<iArgc; i++)
        {
            printf("    %s\n", iArgv[i].c_str());
        }
        printf("******************\n");
    }
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
"\n"
"        --asm=Dialect of arm assembly to write for the --dump option. Either \"gas\" (GNU as) or \"armas\" (RVCT as: default) - Unsupported\n"
"        --e32tran=Translate E32 image --e32input=<inputfile> --output=<outputfile> - Unsupported\n"
"        --exportautoupdate=Auto update output def file according to the input elf file - Unsupported\n"
"        --header: Generate C++ header file for dynamic linking.\n"
"        --man: Describe advanced usage new features.\n"
"        --verbose: Display the operations inside elf2e32.\n"
"        --force: Force E32Image build. All error checks off.\n"
"        --help: This command.\n"
;

void Help()
{
    if(DisableLongVerbosePrint())
        return;
    ToolVersion v;
    ReportLog("\nSymbian Post Linker, Elf2E32 v %d.%d (Build %d)\n",
              (int16_t)v.iMajor, (int16_t)v.iMinor, (int16_t)v.iBuild);
    ReportLog("Usage:  elf2e32 [options] [filename]\n\n");
    ReportLog("Options:\n");
    ReportLog(ScreenOptions);
}

bool operator==(const Opts* left, const Opts& right)
{
    if(left->name != right.name)
        return false;
    if(left->arg != right.arg)
        return false;
    if(left->val != right.val)
        return false;
    if(left->binary_arg1 != right.binary_arg1)
        return false;
    if(left->binary_arg2 != right.binary_arg2)
        return false;
    return true;
}

void ArgParser::ArgInfo(const Opts& opt) const
{
    if(!VerboseOutput ||  DisableLongVerbosePrint())
        return;

    ReportLog("Got arg: --" + opt.name);
    if(!opt.arg.empty())
        ReportLog(" with opt: " + opt.arg);
    ReportLog("\n");

    if(!iTests)
        return;
    const struct Opts *optptr = (struct Opts*)iTests;
    while(!optptr->name.empty())
    {
        if(optptr == opt)
            break;
        optptr++;
    }
    if(optptr->name.empty())
    {
        std::stringstream tmp("Maches missed for option: --");
        tmp << opt.name << std::hex;
        if(!opt.arg.empty())
            tmp << " with arg: " << opt.arg;
        if(opt.binary_arg1 != (uint32_t)-1)
            tmp << " with binary arg: " << opt.binary_arg1;
        if(opt.binary_arg2 != (uint32_t)-1)
            tmp << "; " << opt.binary_arg2 << ";   ";
        ReportError(ErrorCodes::ZEROBUFFER, tmp.str());
    }
}
