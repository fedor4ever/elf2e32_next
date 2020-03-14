#include <string>
#include <iostream>

#include "symbol.h"
#include "deffile.h"
#include "elf2e32_opt.hpp"
#include "symbolprocessor.h"

using namespace std;

string sysdef("Asym1,0;asym2,1;asym3,35;AES_cbc_encrypt,4;AES_cfbr_encrypt_block,25;");
string defin("libcryptou.def");
string defout("target.def");

int main()
{
    Args build_args;
    build_args.iSysdef = sysdef;
    build_args.iDefinput = defin;
    build_args.iDefoutput = defout;

    SymbolProcessor* task = new SymbolProcessor(nullptr, &build_args);
    Symbols result = task->Process();
    delete task;

    DefFile def;
    def.WriteDefFile(defout.c_str(), result);

    for(auto x: result)
    {
        cout << "symbol name: " << x->AliasName() << "\thas ordinal: " << x->Ordinal() << "\n";
    }
    cout << "Hello world!" << endl;
    return 0;
}
