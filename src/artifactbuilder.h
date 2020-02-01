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

class Args;


class ArtifactBuilder : public Task
{
    public:
        ArtifactBuilder(Args* param);
        virtual ~ArtifactBuilder();

    private:
        Args* iOpts = nullptr;
};

#endif // ARTIFACTBUILDER_H
