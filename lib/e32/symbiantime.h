#ifndef SYMBIANTIME_H
#define SYMBIANTIME_H

#include <stdint.h>

class SymbianTime
{
    public:
        SymbianTime();
        uint32_t TimeLo() const;
        uint32_t TimeHi() const;
    private:
        uint32_t iTimeLo = 0;
        uint32_t iTimeHi = 0;
};

#endif // SYMBIANTIME_H
