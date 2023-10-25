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

#ifndef CMDLINEPROCESSOR_H_INCLUDED
#define CMDLINEPROCESSOR_H_INCLUDED

#include <string>

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
	EVar,  // exports "_Z17VariantInitialisev,1"
	EVar2, // exports "VariantInitialise,1"
	EExexp,
	EStdExe, //UID1 - 0x1000007a UID2 - 0x20004C45
	EStdDll, //UID1 - 0x10000079 UID2 - 0x20004C45
	//ecom plugins
	/// TODO (Administrator#1#03/29/20): set right export at ord1
	EAni = 0x10003b22, // animation plugin and uid2
	EFsy = 0x100039df, // file system plugin and uid2
	ELdd = 0x100000af, // logical device driver with uid2 and exports "_Z19CreateLogicalDevicev,1"
	EPlugin = 0x10009D8D, // ECom plug-in and uid2
	EPlugin3 = 0x10009D93, // ECom plug-in and uid2
	EPdd = 0x100039d0, // physical device driver with uid2 and exports "_Z20CreatePhysicalDevicev,1"
	EPdl = 0x10003b1c, // printer device plugin and uid2
	ETextNotifier2 = 0x101fe38b, // notifier for text window server plugin and uid2

/* eka 1 targets. T9 plugin on N-gage QD for example.
Mentioned in SDK doc "Polymorphic DLL targettype summary".
No use cases such target type with that UID2 in Symbian 3 sources.
On Nokia 5800:
 - aknfepuiinterface.dll - uid2 0x1000008d, dll
 - aknfepuiavkonplugin.dll - EPlugin
 - T9 langs - EPlugin
 - T9Core.dll - EPlugin
 - t9rel.dll - uid2 0x1000008d, dll
 On N-gage QD:
 - T9fep.fep - EPlugin

 EFep target with UID2 0x10005e32 in SDK mmp.pm marked as deprecated
 but mentioned only once in SDK as valid. SDK tool has precedency over docs.
 So it deprecated too.
*/
	EFep = 0x10005e32, // front-end processor plugin and uid2
	//end unused

	EFalseTartget
};

uint64_t ProcessCapabilities(const std::string& str);
Paging GetPaging(const std::string& fromArgument);
uint32_t GetFpuType(const std::string& fromArgument);
uint32_t SetToolVersion(const std::string& str);
TargetType GetTarget(const std::string& fromArgument);
void VarningForDeprecatedUID(uint32_t UID2);

#endif // CMDLINEPROCESSOR_H_INCLUDED
