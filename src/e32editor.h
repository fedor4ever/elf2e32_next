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
// Contributors:
//
// Description:
// Generate checksums for the E32Image sections
//
//

#ifndef E32EDITOR_H
#define E32EDITOR_H

#include <stdint.h>

class E32Parser;
class E32ImageHeader;

class E32Editor
{
        E32Editor(const E32Parser* const file);
        void ConstructL();
    public:
        static E32Editor* NewL(const E32Parser* const file);
        ~E32Editor();
        void SetE32Time(uint32_t timeLo, uint32_t timeHi);
    public: //return E32Image header fields
        uint32_t TimeLo() const;
        uint32_t TimeHi() const;
    public: //return CRC for E32Image sections
        uint32_t FullImage() const;
        uint32_t Header() const;
        uint32_t ExportBitMap() const;
        uint32_t Code() const;
        uint32_t Data() const;
        uint32_t Exports() const;
        uint32_t Symlook() const;
        uint32_t Imports() const;
        uint32_t CodeRelocs() const;
        uint32_t DataRelocs() const;
    private:
        E32ImageHeader* iHeader = nullptr;
        const E32Parser* iFile = nullptr;
        const char* iE32File = nullptr;
};

#endif // E32EDITOR_H
