#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <string>

enum ErrorCodes
{
    UNKNOWNERROR,
    MISSEDARGUMENT,
    UNKNOWNOPTION
};

const std::string empty = std::string();
void ReportError(  const ErrorCodes err, const std::string& str = empty, const int = 0);
void ReportWarning(const ErrorCodes err, const std::string& str = empty, const int = 0);

#endif // COMMON_HPP_INCLUDED
