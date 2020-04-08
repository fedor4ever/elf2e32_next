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
// Creates and fills E32Image header fields from command line arguments.
//
// These sections allocated: E32ImageHeader, E32ImageHeaderJ and
// E32ImageHeaderV without field E32ImageHeaderV::iExportDesc
// because it allocates later.
//
//

#ifndef E32HEADERBUILDER_H
#define E32HEADERBUILDER_H

#include "e32file.h"

struct Args;

class E32HeaderBuilder
{
    public:
        E32HeaderBuilder(const Args* opts);
        ~E32HeaderBuilder() {}
        E32SectionUnit MakeE32Header();

    private:
        E32SectionUnit iHeader;
        const Args* iHeaderData = nullptr;
};

#endif // E32HEADERBUILDER_H
