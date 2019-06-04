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
// Control workflow for the elf2e32 tool
//
//

#ifndef ELF2E32_H
#define ELF2E32_H

class Args;
class Task;
class ArgParser;
class E32ImageHeader;

class Elf2E32
{
    public:
        Elf2E32(int argc, char** argv);
        Elf2E32(const Elf2E32&) = delete;
        ~Elf2E32();
        void Run();

    private:
        E32ImageHeader* iHdr = nullptr;
        ArgParser* iArgs = nullptr;
        Args* iArg = nullptr;
        Task* iTask = nullptr;
};

#endif // ELF2E32_H
