// Copyright (c) 2023-2024 Strizhniou Fiodar
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
//  Parse command options
//
//

#include "common.hpp"
#include "getopt.hpp"
#include "getopt_opts.h"
#include "elf2e32_opt.hpp"

bool NeedRawArg(Flags::Flags flag)
{
    if(flag == Flags::CASE_SENSITIVE)
        return true;
    return false;
}

struct Opts getopt(const std::string& argc)
{
    Opts opt;
    if(argc[0] != '-')
    {
        opt.name = argc;
        opt.arg = "Option expected \'-\'" + argc + "for " + argc;
        opt.val = not_recognized_arg;
        return opt;
    }
    if(argc[1] != '-')
    {
        opt.name = argc;
        opt.arg = "Short option handler not implemented!";
        opt.val = not_recognized_arg;
        return opt;
    }

    auto option(argc.substr(2));
    auto argpos = option.find_first_of("=");
    if(argpos != std::string::npos)
    {
        opt.name = option.substr(0, argpos);
        opt.arg = option.substr(argpos+1);
    }
    else
        opt.name = option;

    struct option *optptr = (struct option *) long_opts;
    while(optptr->name != nullptr)
    {
        if(opt.name == optptr->name) {
            opt.val = optptr->val;
            if(!NeedRawArg(optptr->flag))
                opt.arg = ToLower(opt.arg);
            break;
        }
		optptr++;
    }

//no matching option found
	if (optptr->name == nullptr) {
        opt.arg = "unknown option";
        opt.val = OptionsType::ENOTRECOGNIZEDARG;
        return opt;
	}

	switch (optptr->has_arg)
	{
	case no_argument:
	    if(!opt.arg.empty())
        {
            opt.arg = "not recognized arg";
            opt.val = OptionsType::ENOTRECOGNIZEDARG;
        }
		break;
	case required_argument:
	    if(opt.arg.empty())
        {
            opt.arg = "missed required arg";
            opt.val = OptionsType::EMISSEDARG;
        }
		break;
	case optional_argument:
		break;
	default:
	    opt.arg = "unknown argument option for option";
        opt.val = OptionsType::ENOTRECOGNIZEDARG;
		break;
	}
    return opt;
}
