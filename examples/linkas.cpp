#include <iomanip>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;
string LinkAsUID(std::string UID);
string DeduceDLLName(const std::string name, const std::string UID3, const std::string version);
string DSOName(const string& linkAs);

const string dllName = "basicdll.dll";
const string version = "10.1";
const string version0 = "10.0";

uint32_t SetToolVersion(const char* str);
string NormalizeToolVersion(const char* str);

int main()
{
    cout << "Hello world!" << endl;
    cout << LinkAsUID("2000") << " ... " << LinkAsUID("0xe84e1a0e") << " ... " << LinkAsUID("0x0") << endl;
    cout << LinkAsUID("0x01") << endl;

    string tmp = DeduceDLLName(dllName, "2000", version);
    cout << "Linkas: " << tmp << "; DSO name: " << DSOName(tmp) << endl;
    cout << version <<": " <<  NormalizeToolVersion(version.c_str()) << "; " << version0 <<
             ": " << NormalizeToolVersion(version0.c_str()) << endl;

    tmp = DeduceDLLName(dllName, "2000", version);
    cout << "Playing with DLLs\n";
    cout << "Input: " << dllName << " " << "2000" << " " << version << " tansformed to: " << DeduceDLLName(dllName, "2000", version) << endl;

    return 0;
}

std::string LinkAsUID(std::string UID)
{
//    if(UID.size() > 10) reporterror();
//    if(UID.size() == 9) reporterror();
//    if((UID.size() == 10) && (UID[0] != '0') && ((UID[1] != 'x') || (UID[1] != 'X'))) reporterror();
    stringstream buf;

//    strtoul() return zero for zero strings("0x00") and no valid conversion could be performed
//    but zero UID is valid
    if((UID[1] == 'x') || (UID[1] == 'x'))
    {
        buf << "[";
        size_t count = std::count_if( UID.begin(), UID.end(), []( char c ){return c == '0';});
        // first comes defaull zero hex UID
        if((count + 1) == UID.size())
        {
            buf << "00000000" << "]";
            return buf.str();
        }
        // shertened zero hex UID
        if(UID.size() < 10)
        {
            string tmp;
            tmp.append(10 - UID.size(), '0');
            tmp += UID.substr(2, UID.size());
            buf << tmp << "]";
            return buf.str();
        }
        // fully qualified hex UID
        if(UID.size() == 10)
        {
            buf << UID.substr(2, UID.size()) << "]";
            return buf.str();
        }
        //reporterror()
    }

    uint32_t iUid3 = strtoul(UID.c_str(), nullptr, 0);
//    no valid conversion could be performed, a zero value is returned
    if(iUid3 == 0)
        cout << "no valid conversion could be performed, a zero value is returned\n";
//        reporterror();
    if((iUid3 == ULONG_MAX) && (errno == ERANGE))
        cout << " value read is out of the range\n";
//        reporterror();
    buf << "[" << setw(8) << hex << std::setfill('0') << iUid3 << "]";
    return buf.str();
}

string DSOName(const string& linkAs)
{
    string tmp = linkAs;
    tmp.erase(tmp.find_first_of("["));
    tmp += ".dso";
    return tmp;
}

uint32_t SetToolVersion(const char* str)
{
	uint32_t hi, lo;
	hi = std::stoi(str);
    string t(str);
    lo = std::stoi(t.substr( t.find_first_of(".,;") + 1 ));
    return ((hi & 0xFFFF) << 16) | (lo & 0xFFFF);
}

string NormalizeToolVersion(const char* str)
{
    stringstream buf;
    buf << "{" << setw(8) << hex << std::setfill('0') << SetToolVersion(str) << "}";
    return buf.str();
}

string DeduceDLLName(const std::string name, const std::string UID3, const std::string version)
{
    string tmp = name;
    tmp.insert(tmp.find_last_of("."), NormalizeToolVersion(version.c_str()));
    tmp.insert(tmp.find_last_of("."), LinkAsUID(UID3));
    return tmp ;
}
