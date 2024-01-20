// Copyright (c) 2024 Strizhniou Fiodar
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
//  It creates CRC32 checksums for DSO files and store them in file with ".dcrc" extension.
//  Also verify generated DSO between different versions if precalculated file used.
//
//

#ifndef DSOCRCProcessor_H
#define DSOCRCProcessor_H

#include <string>

#include "crcprocessor.h"

struct Args;

struct CRCData
{
    uint32_t iDSOCrc = -1;
};

class DSOCRCProcessor: public CRCProcessor
{
    public:
        DSOCRCProcessor(const Args* arg);
        virtual ~DSOCRCProcessor();
    private:
        virtual void CRCFromFile() final override;
        virtual void SetCRCFiles() final override;
        virtual std::string CRCAsStr() final override;
        virtual void CRCFile(std::string& s) final override;
        virtual bool PrintInvalidTargetCRC() final override;
        virtual void ProcessTokens(const std::string&, uint32_t crc) final override;
    private:
        CRCData iCRCIn;
        CRCData iCRCOut;

};

#endif // DSOCRCProcessor_H
