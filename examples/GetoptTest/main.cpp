#include <random>  // std::mt19937; needed to feed std::shuffle()
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <exception>

#include "getopt.h"
#include "common.hpp"
#include "argparser.h"
#include "test_options.h"
#include "elf2e32_opt.hpp"

using namespace std;

void Help();

const string man =
"This program is designed to create E32Image and related files (--man-build)."
" In addition, the program can change some properties of E32Image (--man-edit).\n"
;

const string manBuild =
"Creating E32Image, def, dso and import header (hereinafter referred "
"to as artifacts) is represented by 2 modes:\n"
"\t* receiving a list of exported symbols from dso and creating def, import header (--man-build-dsodump)\n"
"\t* creating artifacts from def, sysdef, elf (--man-build-artifacts).\n"
;

const string manDsoDump =
" Options --elfinput and --defoutput are used together to enable the creation of artifacts from .dso mode,"
" --elfinput is used to load data, assembly artifacts are specified via --header and --defoutput\n"
;

const string manArtifacts =
"The creation of E32Image and other artifacts is controlled by the corresponding flags."
" To create some artifacts, you need to set certain command line parameters:\n"
"\t* --output and --elfinput are required to create an E32Image, for dll and exedll the"
" following parameters can be specified --sysdef, --definput.\n"
"\t* At least one of the following parameters is required: --elfinput, --sysdef,"
" --definput, to create any of them: .def, .dso, import header.\n"
;

const string manEdit =
"Two flags are required to modify E32Image: --e32input and --output."
" You can change the following file properties: compression algorithm, UIDs,"
" Heap committed and reserved size, specify the process priority for your"
" executable EXE, set new capabilities while the file save time will become"
" the file creation time, and the current version of elf2e32 will become the version of the"
" utility that created the file.\n"
;

static bool VerboseOutput;

void ArgInfo(const Opts& opt);
bool ArgParse(Args* arg, const std::vector<std::string>& cmdline);

int main(int argc, char** argv)
{
    uint16_t tests_failed = 0;
    cout << "Running cmdline otions tests!" << endl;
    Args* arg = new Args();
    std::vector<std::string> tmp(option_args);
    for(uint16_t i = 0; i < 1000; i++)
    {
        std::shuffle(tmp.begin(), tmp.end(),
               std::mt19937{std::random_device{}()});
        try{
            ArgParser* parser = new ArgParser(tmp, options);
            parser->Parse(arg);
            delete parser;
//            cout << "Test " << i << " succeded for args:\n";
//            for(auto x: tmp)
//                cout << x << " ";
            cout << "\n";
        }
        catch(std::exception const& ex)
        {
            tests_failed++;
            cout << "Reason failure: " << ex.what() << '\n';
            cout << "Test " << i << " failure for args:\n";
            for(auto x: tmp)
                cout << x << " ";
            cout << "\n\n";
        }
        catch(...)
        {
            tests_failed++;
            cout << "Test " << i << " failure for args:\n";
            for(auto x: tmp)
                cout << x << " ";
            cout << "\n";
        }
    }
    if(tests_failed)
        cout << "Tests failed: " << tests_failed << "\n";
    else
        cout << "All test passed!\n";
    delete arg;
    return 0;
}

//void Help() {}

void ReportLog(const std::string& str, int x, int y, int z)
{
    cout << str;
    if(x != -1)
        cout << x << " ";
    if(y != -1)
        cout << y << " ";
    if(z != -1)
        cout << z;
    cout << "\n";
}

void ReportError(const ErrorCodes err, const std::string& str, void (*f)())
{
    cout << "Reason failure: " + str + "\n";
    throw err;
}


void ReportError(const ErrorCodes err, const std::string& str,
            const std::string& s, const int)
{
    cout << "Reason failure: " + str + s << "\n";
    throw err;
}

string ToLower(const string& s)
{
    string data = s;
    std::transform(data.begin(), data.end(), data.begin(),
    [](unsigned char c){ return std::tolower(c); });
    return data;
}
