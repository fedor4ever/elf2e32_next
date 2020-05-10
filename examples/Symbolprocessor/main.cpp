// Copyright (c) 2020 Strizhniou Fiodar.
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Description:
// tests for elf2e32
//
//

#include <string>
#include <memory>
#include <iostream>

#include "symbol.h"
#include "deffile.h"
#include "elfparser.h"
#include "elf2e32_opt.hpp"
#include "symbolprocessor.h"

using namespace std;

string sysdef("Asym1,0;asym2,1;asym3,25;AES_cbc_encrypt,4;x509_name_ff,3987;");
string defin("libcryptou.def");
string defout("target.def");
string elffile("libcrypto.dll");

void SymbolChecker(const Symbols& s);
void TestBody(const Args& build_args);

// only applied to test1() and test2() because they swap symbols
void CheckSymbol(const Symbol* s, string name, uint32_t ord)
{
    if(s->AliasName() != name)
        return;

    if(s->Ordinal() == ord)
        ReportLog(name.append(" symbol got right ordinal: %d\n\n"), s->Ordinal());
    else
        ReportLog(name.append(" symbol got wrong ordinal: %d\n\n"), s->Ordinal());
}

Args initArgs(string sysdef, string defin, string defout = string(),
              string elffile = string(), bool unfrozen = false)
{
    Args build_args;
    build_args.iSysdef    = sysdef;
    build_args.iDefinput  = defin;
    build_args.iDefoutput = defout;
    build_args.iElfinput  = elffile;
    build_args.iUnfrozen = unfrozen;
    return build_args;
}

void Success()
{
    cout <<  "and succeed\n\n";
}
void test1()
{
    cout << "test1: Full build with frozen exports should fail... \n";
    Args build_args = initArgs(sysdef, defin, defout, elffile);
    TestBody(build_args);
    cout << "but succeed =P \n";
}

void test2()
{
    cout << "test2: Full build with unfrozen exports shouldn't fail... \n";
    Args build_args = initArgs(sysdef, defin, defout, elffile, true);
    TestBody(build_args);
    Success();
}

void test3()
{
    cout << "test3: Build with unfrozen exports and without sysdef shouldn't fail... \n";
    Args build_args = initArgs(string(), defin, defout, elffile, true);
    TestBody(build_args);
    Success();
}

void test4()
{
    cout << "test4: Build with unfrozen exports and without sysdef"
    " and defout shouldn't fail... \n";
    Args build_args = initArgs(string(), defin, string(), elffile, true);
    TestBody(build_args);
    Success();
}

void test5()
{
    cout << "test5: Build with frozen exports and without sysdef"
    " and defout shouldn't fail... \n";
    Args build_args = initArgs(string(), defin, string(), elffile);
    TestBody(build_args);
    Success();
}

void test6()
{
    cout << "test6: Build with frozen exports from elffile shouldn't fail... \n";
    Args build_args = initArgs(string(), string(), string(), elffile);
    TestBody(build_args);
    Success();
}

void test7()
{
    cout << "test7: Full build with frozen exports should fail... \n";
    Args build_args = initArgs(sysdef, defin, defout);
    TestBody(build_args);
    cout << "but succeed\n";
}

void test8()
{
    cout << "test8: Build with unfrozen exports shouldn't fail... \n";
    Args build_args = initArgs(sysdef, defin, defout, string(), true);
    TestBody(build_args);
    Success();
}

void test9()
{
    cout << "test9: Build with frozen exports shouldn't fail... \n";
    Args build_args = initArgs(sysdef, string(), defout);
    TestBody(build_args);
    Success();
}

void test10()
{
    cout << "test10: Build with frozen exports shouldn't fail... \n";
    Args build_args = initArgs(string(), defin, defout);
    TestBody(build_args);
    Success();
}

void test11()
{
    cout << "test11: Build with frozen exports should fail... \n";
    Args build_args = initArgs(sysdef, string(), defout, elffile);
    TestBody(build_args);
    cout << "but succeed\n";
}

void test12()
{
    cout << "test12: Build with frozen exports shouldn't fail... \n";
    Args build_args = initArgs(sysdef, string(), defout, elffile, true);
    TestBody(build_args);
    Success();
}

void Fail()
{
    cout << "Test failed!\n";
}

void TestBody(const Args& build_args)
{
    ElfParser* iElfParser = nullptr;

    if(!build_args.iElfinput.empty())
    {
        iElfParser = new ElfParser(build_args.iElfinput);
        iElfParser->GetElfFileLayout();
    }

    SymbolProcessor* task = new SymbolProcessor(&build_args, iElfParser);
    Symbols result = task->GetExports();

    DefFile def;
    def.WriteDefFile(defout.c_str(), result);

    SymbolChecker(result);

    delete task;
    if(iElfParser)
        delete iElfParser;
}

typedef void (*test)();
test tests[] = {test1, test2, test3, test4,
                test5, test6, test7, test8,
                test9, test10, test11, test12,
                nullptr};

size_t length = sizeof(tests)/sizeof(tests[0]);

int main()
{
    for(size_t i = 0; i < length; i++)
    {
        if(!tests[i])
            break;
        try
        {
            tests[i]();
        }
        catch( ... )
        {
            Fail();
        }
    }
    cout << "Tests finished!" << endl;
    return 0;
}

void SymbolChecker(const Symbols& s)
{
    for(auto x: s)
    {
        if(x->AliasName() == "Asym1")
            ReportLog("Found ignoredf symbol: Asym1");

        CheckSymbol(x, "asym2", 1);
        CheckSymbol(x, "ACCESS_DESCRIPTION_free", 3988);

        CheckSymbol(x, "AES_cbc_encrypt", 4);
        CheckSymbol(x, "AES_cfb128_encrypt", 3989);

        CheckSymbol(x, "asym3", 25);
        CheckSymbol(x, "ASN1_BMPSTRING_free", 3990);

        CheckSymbol(x, "x509_name_ff", 3987);
//        CheckSymbol(x, "", );
//        cout << "symbol name: " << x->AliasName() << "\thas ordinal: " << x->Ordinal() << "\n";
//        cout << " symbol value: " << x->Elf_st_value() << "\n";
    }
}
