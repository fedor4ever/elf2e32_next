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

#ifndef E32FILE_H
#define E32FILE_H

class Args;
class ElfParser;

class E32File
{
    public:
        E32File(const Args* args, const ElfParser* elfParser);
        ~E32File();
        void WriteE32File();
    private:
        const Args* iE32Opts = nullptr;
        const ElfParser* iElfSrc = nullptr;
};

#endif // E32FILE_H
