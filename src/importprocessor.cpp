#include "importprocessor.h"

ImportProcessor::ImportProcessor(const ElfParser* elf): iElf(elf)
{
    //ctor
}

ImportProcessor::~ImportProcessor()
{
    //dtor
}

uint32_t ImportProcessor::DllCount() const
{
    return 12; // stub for tests sake
}

E32Section ImportProcessor::Imports()
{
    E32Section imps;
    return imps;
}
