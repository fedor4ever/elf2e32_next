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
// Build E32Image and other companions
//
//

#ifndef ARTIFACTBUILDER_H
#define ARTIFACTBUILDER_H

#include <list>
#include <vector>
#include "task.hpp"

class Args;
class Symbol;
class ElfParser;

typedef std::list <Symbol*>	Symbols;

class ArtifactBuilder: public Task
{
    public:
        ArtifactBuilder(Args* param);
        virtual ~ArtifactBuilder();
        void Run() override;
    private:
        void PrepareBuild();
        void MakeDSO();
        void MakeDef();
        void MakeE32();
        void DsoImpLibName();
    private:
        Args* iOpts = nullptr;
        ElfParser* iElfParser = nullptr;
        Symbols iSymbols;
        std::vector<std::string> iDsoImpLibName;
};

#endif // ARTIFACTBUILDER_H
