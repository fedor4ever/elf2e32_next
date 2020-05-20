#ifndef IMPORTPROCESSOR_H
#define IMPORTPROCESSOR_H

#include "e32file.h"

class ImportProcessor
{
    public:
        ImportProcessor(const ElfParser* elf);
        ~ImportProcessor();
        uint32_t DllCount() const;
        E32Section Imports();

    private:
        const ElfParser* iElf = nullptr;
};

#endif // IMPORTPROCESSOR_H
