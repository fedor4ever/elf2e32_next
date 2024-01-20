// Copyright (c) 2023 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Description:
//  Hold accepted command options
//
//

#ifndef GETOPT_OPTS_H_INCLUDED
#define GETOPT_OPTS_H_INCLUDED

#include "getopt.hpp"
#include "elf2e32_opt.hpp"

namespace Flags{
enum Flags
{
    NONE = 0,
    CASE_SENSITIVE = 1,
};
}

static struct option long_opts[] =
{
    {"uid1",  required_argument,  Flags::NONE, OptionsType::EUID1},
    {"uid2",  required_argument,  Flags::NONE, OptionsType::EUID2},
    {"uid3",  required_argument,  Flags::NONE, OptionsType::EUID3},
    {"sid",   required_argument,  Flags::NONE, OptionsType::ESID},
    {"vid",   required_argument,  Flags::NONE, OptionsType::EVID},
    {"heap",  required_argument,  Flags::NONE, OptionsType::EHEAP},
    {"stack", required_argument,  Flags::NONE, OptionsType::ESTACK},
    // for E32ImageHeader::iFlags
    {"fixedaddress",     no_argument,  Flags::NONE, OptionsType::EFIXEDADDRESS},
    {"callentry",        no_argument,  Flags::NONE, OptionsType::ECALLENTRY},
    {"fpu",        required_argument,  Flags::NONE, OptionsType::EFPU},
    {"codepaging", required_argument,  Flags::NONE, OptionsType::ECODEPAGING},
    {"datapaging", required_argument,  Flags::NONE, OptionsType::EDATAPAGING},
    {"paged",            no_argument,  Flags::NONE, OptionsType::EPAGED},
    {"defaultpaged",     no_argument,  Flags::NONE, OptionsType::EDEFAULTPAGED},
    {"unpaged",          no_argument,  Flags::NONE, OptionsType::EUNPAGED},
    {"debuggable",       no_argument,  Flags::NONE, OptionsType::EDEBUGGABLE},
    {"smpsafe",          no_argument,  Flags::NONE, OptionsType::ESMPSAFE},
    // for image generation
    {"targettype",   required_argument,  Flags::NONE, OptionsType::ETARGETTYPE},
    {"linkas",       required_argument,  Flags::CASE_SENSITIVE, OptionsType::ELINKAS},
    {"uncompressed",       no_argument,  Flags::NONE, OptionsType::EUNCOMPRESSED},
    {"compressionmethod", required_argument,  Flags::NONE, OptionsType::ECOMPRESSIONMETHOD},
    {"unfrozen",           no_argument,  Flags::NONE, OptionsType::EUNFROZEN},
    {"ignorenoncallable",  no_argument,  Flags::NONE, OptionsType::EIGNORENONCALLABLE},
    {"capability",   required_argument,  Flags::NONE, OptionsType::ECAPABILITY},
    {"sysdef",       required_argument,  Flags::CASE_SENSITIVE, OptionsType::ESYSDEF},
    {"dlldata",            no_argument,  Flags::NONE, OptionsType::EDLLDATA},
    {"priority",     required_argument,  Flags::NONE, OptionsType::EPRIORITY},
    {"excludeunwantedexports",  no_argument,  Flags::NONE, OptionsType::EEXCLUDEUNWANTEDEXPORTS},
    {"customdlltarget",    no_argument,  Flags::NONE, OptionsType::ECUSTOMDLLTARGET},
    {"namedlookup",        no_argument,  Flags::NONE, OptionsType::ENAMEDLOOKUP},
    // input files
    {"definput",  required_argument,  Flags::CASE_SENSITIVE, OptionsType::EDEFINPUT},
    {"defoutput", required_argument,  Flags::CASE_SENSITIVE, OptionsType::EDEFOUTPUT},
    {"elfinput",  required_argument,  Flags::CASE_SENSITIVE, OptionsType::EELFINPUT},
    {"output",    required_argument,  Flags::CASE_SENSITIVE, OptionsType::EOUTPUT},
    {"dso",       required_argument,  Flags::CASE_SENSITIVE, OptionsType::EDSO},
    {"libpath",   required_argument,  Flags::CASE_SENSITIVE, OptionsType::ELIBPATH},
    {"e32input",  required_argument,  Flags::CASE_SENSITIVE, OptionsType::EE32INPUT},
    {"header",    optional_argument,  Flags::CASE_SENSITIVE, OptionsType::EHEADER},
    // info for E32 image
    {"dump",      required_argument,  Flags::NONE, OptionsType::EDUMP},
    // common options
    {"log",             required_argument,  Flags::CASE_SENSITIVE, OptionsType::ELOG},
    {"version",         required_argument,  Flags::NONE, OptionsType::EVERSION},
    {"man",                   no_argument,  Flags::NONE, OptionsType::EMAN},
    {"man-edit",              no_argument,  Flags::NONE, OptionsType::EMANEDIT},
    {"man-build",             no_argument,  Flags::NONE, OptionsType::EMANBUILD},
    {"man-build-dsodump",     no_argument,  Flags::NONE, OptionsType::EMANDSODUMP},
    {"man-build-artifacts",   no_argument,  Flags::NONE, OptionsType::EMANARTIFACTS},
    {"help",                  no_argument,  Flags::NONE, OptionsType::EHELP},
    // dev options
    {"filecrc",         optional_argument,  Flags::CASE_SENSITIVE, OptionsType::FILECRC},
    {"time",            required_argument,  Flags::NONE, OptionsType::TIME},
    {"verbose",         optional_argument,  Flags::NONE, OptionsType::VERBOSE},
    {"force",                 no_argument,  Flags::NONE, OptionsType::FORCEE32BUILD},
    // ignored options
    {"messagefile",     required_argument,  Flags::CASE_SENSITIVE, OptionsType::EMESSAGEFILE},
    {"dumpmessagefile", required_argument,  Flags::CASE_SENSITIVE, OptionsType::EDUMPMESSAGEFILE},
    {nullptr,0, Flags::NONE,0}
};

#endif // GETOPT_OPTS_H_INCLUDED
