#include <iostream>
#include "symbiantime.h"

using namespace std;

constexpr uint64_t daysTo2000AD=730497;
int64_t timeToInt64(time_t aTime)
{
	aTime-=(30*365*24*60*60+7*24*60*60);	// seconds since midnight Jan 1st, 2000
	int64_t t=daysTo2000AD*24*3600;	// seconds since 0000
	t+=aTime;	// seconds since 0000
	t=t+3600;								// BST (?)
	return t*1000000;						// milliseconds
}

int main()
{
//    time_t t = time(nullptr);
    time_t t = 1594932463;
	int64_t ltime = timeToInt64(t);
	int32_t iTimeLo=(uint32_t)ltime;
	int32_t iTimeHi=(uint32_t)(ltime>>32);
    cout << "Hello time! " << iTimeHi << " " << iTimeLo << endl;

    SymbianTime st(t);
    cout << "Hello time! " << st.TimeHi() << " " << st.TimeLo() << endl;
    return 0;
}
