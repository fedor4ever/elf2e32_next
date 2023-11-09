// Copyright (c) 2023 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.

#ifndef GETOPT_OPTS_H_INCLUDED
#define GETOPT_OPTS_H_INCLUDED

#include "getopt.hpp"
#include "elf2e32_opt.hpp"

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
    {"man-build-dsodump",     no_argument, nullptr, OptionsType::EMANDSODUMP},
    {"man-build-artifacts",   no_argument, nullptr, OptionsType::EMANARTIFACTS},
    {"help",                  no_argument, nullptr, OptionsType::EHELP},
    // dev options
    {"filecrc",         optional_argument, nullptr, OptionsType::FILECRC},
    {"time",            required_argument, nullptr, OptionsType::TIME},
    {"verbose",         optional_argument, nullptr, OptionsType::VERBOSE},
    {"force",                 no_argument, nullptr, OptionsType::FORCEE32BUILD},
    // ignored options
    {"messagefile",     required_argument, nullptr, OptionsType::EMESSAGEFILE},
    {"dumpmessagefile", required_argument, nullptr, OptionsType::EDUMPMESSAGEFILE},
    {nullptr,0,nullptr,0}
};

#endif // GETOPT_OPTS_H_INCLUDED
