#ifndef E32EDITOR_H
#define E32EDITOR_H

#include <stdint.h>

class E32Parser;

class E32Editor
{
    public:
        E32Editor(const E32Parser* file);
        ~E32Editor();
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
        char* Duplicate();
    private:
        E32Parser* iFile = nullptr;
        const char* iE32File = nullptr;
};

#endif // E32EDITOR_H
