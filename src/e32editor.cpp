#include "e32editor.h"

E32Editor::E32Editor(const E32Parser* file)
{
    //ctor
}

E32Editor::~E32Editor()
{
    delete[] iE32File;
}

char* E32Editor::Duplicate()
{

}

uint32_t E32Editor::DataRelocs() const
{

}

uint32_t E32Editor::CodeRelocs() const
{

}

uint32_t E32Editor::Imports() const
{

}

uint32_t E32Editor::Symlook() const
{

}

uint32_t E32Editor::Exports() const
{

}

uint32_t E32Editor::Data() const
{

}

uint32_t E32Editor::Code() const
{

}

uint32_t E32Editor::ExportBitMap() const
{

}

uint32_t E32Editor::Header() const
{

}

uint32_t E32Editor::FullImage() const
{

}

uint32_t E32Editor::TimeHi() const
{

}

uint32_t E32Editor::TimeLo() const
{

}

