// Copyright (c) 2018-2020 Strizhniou Fiodar
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
// Implementation of the Class E32Flags for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "e32flags.h"
#include "e32common.h"
#include "elf2e32_opt.hpp"

E32Flags::E32Flags(const Args* args): iArgs(args) {}

E32Flags::~E32Flags(){}

/**< This function sets the named symbol-lookup attribute in the E32 image. */
void E32Flags::SetSymbolLookup()
{
	if(iArgs->iNamedlookup)
		iFlags |= KImageNmdExpData;
	else
		iFlags &= ~KImageNmdExpData;
}

/**<This function sets the FPU type that the E32 image targets.  */
void E32Flags::SetFPU()
{
	iFlags &=~ KImageHWFloatMask;

	if(iArgs->iFpu == TFloatingPointType::EFpTypeVFPv2)
		iFlags |= KImageHWFloat_VFPv2;
    else if(iArgs->iFpu == TFloatingPointType::EFpTypeVFPv3)
		iFlags |= KImageHWFloat_VFPv3;
    else if(iArgs->iFpu == TFloatingPointType::EFpTypeVFPv3D16)
		iFlags |= KImageHWFloat_VFPv3D16;
}

/**< This function sets the Debuggable attribute in the E32 image. */
void E32Flags::SetDebuggable()
{
	if (iArgs->iDebuggable)
		iFlags |= KImageDebuggable;
	else
		iFlags &= ~KImageDebuggable;
}

/**< This function sets the SmpSafe attribute in the E32 image. */
void E32Flags::SetSmpSafe()
{
	if (iArgs->iSmpsafe)
		iFlags |= KImageSMPSafe;
	else
		iFlags &= ~KImageSMPSafe;
}

/**< This function sets the call entry point of the E32 image. */
void E32Flags::SetCallEntryPoints()
{
	if (iArgs->iCallentry)
		iFlags|=KImageNoCallEntryPoint;
	else
		iFlags&=~KImageNoCallEntryPoint;
}

/** \brief This function set most flags appropriate for E32ImageHeader::iFlags.
 *
 * \return value for E32ImageHeader::iFlags initialization.
 *
 * This class doesn't set these flags:
 * #KImageDll - depends from symbol named "_E32Dll" in elf file;
 * #KImageFixedAddressExe - depends from symbol named "_E32Dll" in elf file;
 *
 * This class set these flags:
 * #KImageNmdExpData
 * #KImageHWFloatMask
 * #KImageHWFloat_VFPv2
 * #KImageDebuggable
 * #KImageSMPSafe
 * #KImageNoCallEntryPoint
 * #KImageImpFmt_ELF
 * #KImageABI_EABI
 * #KImageEpt_Eka2
 * #KImageHdrFmt_V
 * #KImageCodePaged
 * #KImageCodeUnpaged
 * #KImageDataPaged
 * #KImageDataUnpaged
 */
uint32_t E32Flags::Run()
{
    // Import format is ELF-derived
	iFlags |= KImageImpFmt_ELF;
	// ABI is ARM EABI
	iFlags |= KImageABI_EABI;
	iFlags |= KImageEpt_Eka2;
    iFlags |= KImageHdrFmt_V;

    void SetSymbolLookup();
    void SetFPU();
    void SetDebuggable();
    void SetSmpSafe();
    void SetCallEntryPoints();
    void SetPaged();

    return iFlags;
}

/**< This function sets the paging for code and data sections of the E32 image. */
void E32Flags::SetPaged()
{
	// Code paging.
	if(iArgs->iCodePaging == Paging::PAGED)
	{
		iFlags |= KImageCodePaged;
		iFlags &= ~KImageCodeUnpaged;
	}
	else if(iArgs->iCodePaging == Paging::UNPAGED)
	{
		iFlags &= ~KImageCodePaged;
		iFlags |= KImageCodeUnpaged;
	}
    else if(iArgs->iCodePaging == Paging::DEFAULT)
	{
		iFlags &= ~KImageCodePaged;
		iFlags &= ~KImageCodeUnpaged;
	}

	// Data paging.
	if(iArgs->iDataPaging == Paging::PAGED)
	{
		iFlags |=  KImageDataPaged;
		iFlags &= ~KImageDataUnpaged;
	}
	else if(iArgs->iDataPaging == Paging::UNPAGED)
	{
		iFlags &= ~KImageDataPaged;
		iFlags |=  KImageDataUnpaged;
	}
    else if (iArgs->iDataPaging == Paging::DEFAULT)
	{
		iFlags &= ~KImageDataPaged;
		iFlags &= ~KImageDataUnpaged;
	}
}
