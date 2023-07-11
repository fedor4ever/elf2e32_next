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
// A collection of functions for retrieving data from command line arguments.
//
//

#include <cstring>

#include "common.hpp"
#include "e32common.h"
#include "e32capability.h"
#include "cmdlineprocessor.h"

using std::string;

uint32_t GetFpuType(const std::string& fromArgument)
{
    string data = ToLower(fromArgument);
    if(data == "softvfp")
        return TFloatingPointType::EFpTypeNone;
    else if(data == "vfpv2")
        return TFloatingPointType::EFpTypeVFPv2;
    else if(data == "vfpv3")
        return TFloatingPointType::EFpTypeVFPv3;
    else if(data == "vfpv3D16")
        return TFloatingPointType::EFpTypeVFPv3D16;
    ReportError(ErrorCodes::ARGUMENTNAME, fromArgument);
    return 0;
}

Paging GetPaging(const std::string& fromArgument)
{
    string data = ToLower(fromArgument);
    if(data == "paged")
        return Paging::PAGED;
    else if(data == "unpaged")
        return Paging::UNPAGED;
    else if(data == "default")
        return Paging::DEFAULT;
    ReportError(ErrorCodes::ARGUMENTNAME, fromArgument);
    return Paging::DEFAULT; //unreachable anyway
}

TargetType GetTarget(const std::string& fromArgument)
{
    string data = ToLower(fromArgument);
    if(data == "ani")
        return TargetType::EAni;
    else if(data == "fsy")
        return TargetType::EFsy;
    else if(data == "plugin")
        return TargetType::EPlugin;
    else if(data == "plugin3")
        return TargetType::EFep;    else if(data == "textnotifier2")
        return TargetType::ETextNotifier2;
    else if(data == "pdl")
        return TargetType::EPdl;
    else if(data == "dll")
        return TargetType::EDll;    else if(data == "kdll")
        return TargetType::EDll; //revisit
    else if(data == "kext")
        return TargetType::EDll; //revisit
    else if(data == "exe")
        return TargetType::EExe;
    else if(data == "epocexe")
        return TargetType::EExe;
    else if(data == "exedll")
        return TargetType::EExe;
    else if(data == "exexp")
        return TargetType::EExexp;    else if(data == "implib")
        return TargetType::EImportLib;
    else if((data == "klib") || (data == "lib"))
        return TargetType::EFalseTartget;
    else if(data == "none")
        return TargetType::EFalseTartget;    else if(data == "ldd")
        return TargetType::ELdd;    else if(data == "pdd")
        return TargetType::EPdd;
    else if(data == "stdexe")
        return TargetType::EStdExe;
    else if(data == "stddll")
        return TargetType::EStdDll;
    else if(data == "var")
        return TargetType::EVar;
    else if(data == "var2")
        return TargetType::EVar2;
// deprecated targets
    else if((data == "app") || (data == "ctl") ||
            (data == "ecomiic") || (data == "mda") ||
            (data == "mdl") || (data == "notifier") ||
            (data == "rdl") || (data == "opx") ||
            (data == "fep") )
        ReportError(ErrorCodes::DEPRECATEDTARGET, fromArgument);

    ReportError(ErrorCodes::ARGUMENTNAME, fromArgument);
    return TargetType::EInvalidTargetType;
}

void VarningForDeprecatedUID(uint32_t UID2)
{
    if(UID2 == 0x10005e32)
        ReportLog("Unmigrated FEP detected from use of UID 0x10005e32\n");
    if(UID2 == 0x10004cc1)
        ReportLog("Unmigrated Application Initaliser (CEikLibrary deriver) detected from use of UID 0x10004cc1\n");
    if(UID2 == 0x10003a30)
        ReportLog("Unmigrated Conarc plugin detected from use of UID 0x10003a30\n");
    if(UID2 == 0x10003a19)
        ReportLog("Unmigrated Recogniser detected from use of UID 0x10003a19\n");
    if(UID2 == 0x10003a37)        ReportLog("Unmigrated Recogniser detected from use of UID 0x10003a37\n");
    if(UID2 == 0x10003a34)
        ReportLog("Unmigrated CTL detected from use of UID 0x10003a34\n");
}

uint16_t ProcessPriority(const std::string& fromArgument)
{
    string str = ToLower(fromArgument);
	if(str == "low")
		return (uint16_t)TProcessPriority::EPriorityLow;
	if(str == "background")
		return (uint16_t)TProcessPriority::EPriorityBackground;
	if(str == "foreground")
		return (uint16_t)TProcessPriority::EPriorityForeground;
	if(str == "high")
		return (uint16_t)TProcessPriority::EPriorityHigh;
	if(str == "windowserver")
		return (uint16_t)TProcessPriority::EPriorityWindowServer;
	if(str == "fileserver")
		return (uint16_t)TProcessPriority::EPriorityFileServer;
	if(str == "realtimeserver")
		return (uint16_t)TProcessPriority::EPriorityRealTimeServer;
	if(str == "supervisor")
		return (uint16_t)TProcessPriority::EPrioritySupervisor;
	ReportError(ErrorCodes::ARGUMENTNAME, fromArgument);
    return 0;
}

// parse negative capabilities "ALL-TCB-TrustedUI" but "ALL+TCB-TrustedUI" not allowed
uint64_t ProcessALLCapabilities(const std::string& fromArgument)
{
    uint64_t flag = 0;
    const Property* p = capabilities;
    while(p->name != nullptr)
    {
        flag |= p->flag;
        p++;
    }

    string tmp;
    char* token = strtok(fromArgument.c_str(), "-");
    token = strtok(nullptr, "-"); // skip "All" capability
    while(token)
    {
        const Property* p = capabilities;
        while(p->name != nullptr)
        {
            tmp = ToLower(p->name);
            if(tmp == token)
            {
                flag &= ~p->flag;
                token = nullptr;
                break;
            }
            p++;
        }
        if(token)
            ReportError(ErrorCodes::INVALIDARGUMENT, "capability", token);
        token = strtok(nullptr, "-");
    }
    return flag;
}

//! looks like "ALL-TCB-TrustedUI" or "TCB+TrustedUI"
uint64_t ProcessCapabilities(const std::string& fromArgument)
{
    string str = ToLower(fromArgument);

    if(str.substr(0, 4) == "none")
        return 0;
    if(str.substr(0, 3) == "all")
        return ProcessALLCapabilities(str);

    uint64_t flag = 0;
    string tmp;
    char* token = strtok(str.c_str(), "+");

    while(token)
    {
        const Property* p = capabilities;
        while(p->name != nullptr)
        {
            tmp = ToLower(p->name);
            if(tmp == token)
            {
                flag |= p->flag;
                token = nullptr;
                break;
            }
            p++;
        }
        if(token)
            ReportError(ErrorCodes::INVALIDARGUMENT, "capability", token);
        token = strtok(nullptr, "+");
    }
    return flag;
}

uint32_t SetToolVersion(const char* str)
{
	uint32_t hi, lo;
	hi = std::stoi(str);
	if(hi > USHRT_MAX)
        ReportError(ErrorCodes::VALUEOVERFLOW, "--VERSION [major]");
    string t(str);
    lo = std::stoi(t.substr( t.find_first_of(".,;") + 1 ));
	if(lo > USHRT_MAX)
        ReportError(ErrorCodes::VALUEOVERFLOW, "--VERSION [minor]");
    return ((hi & 0xFFFF) << 16) | (lo & 0xFFFF);
}
