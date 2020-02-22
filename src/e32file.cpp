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
// Create E32Image.
//
//

#include "e32file.h"
#include "argparser.h"
#include "elfparser.h"

E32File::E32File(const Args* args, const ElfParser* elfParser):
    iE32Opts(args), iElfSrc(elfParser)
{
    //ctor
}

E32File::~E32File()
{
    //dtor
}

void E32File::WriteE32File()
{
    ;
}

void BuildE32Image(const Args* args, const ElfParser* elfParser)
{
    E32File file(args, elfParser);
    file.WriteE32File();
}
