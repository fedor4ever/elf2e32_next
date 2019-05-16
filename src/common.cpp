#include <iostream>
#include "common.hpp"

using std::cout;

void ReportError(const ErrorCodes err, const std::string& s, const int)
{
    cout << "elf2e32: Error: ";
    switch(err)
    {
    case ErrorCodes::MISSEDARGUMENT:
        cout << "option " << s << " has missed argument\n";
        break;
    case ErrorCodes::UNKNOWNOPTION:
        cout << "option " << s << " is Unrecognized.\n";
        break;
    case ErrorCodes::UNKNOWNERROR: //fallthru
    default:
        cout << "Something bad. wrong option missing!\n";
        break;
    }
}

void ReportWarning(const ErrorCodes err, const std::string& s, const int)
{
    ;
}
