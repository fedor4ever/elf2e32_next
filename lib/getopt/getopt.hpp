// Copyright (c) 2023 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.

#ifndef GETOPT_LONG_H_INCLUDED
#define GETOPT_LONG_H_INCLUDED

#include <string>

struct Opts
{
    std::string name;
    std::string arg;
    int val = -1;
    //struct TestData compatibility
    uint32_t binary_arg1 = -1;
    uint32_t binary_arg2 = -1;
};

struct Opts getopt(const std::string& argc);

struct option		/* specification for a long form option...	*/
{
  const char *name;		/* option name, without leading hyphens */
  int         has_arg;		/* does it take an argument?		*/
  int        *flag;		/* where to save its status, or NULL	*/
  int         val;		/* its associated status value		*/
};

enum    		/* permitted values for its `has_arg' field...	*/
{
  no_argument = 0,      	/* option never takes an argument	*/
  required_argument,		/* option always requires an argument	*/
  optional_argument		/* option may take an argument		*/
};

enum
{
    missed_arg = -1,
    not_recognized_arg = -2
};

#endif // GETOPT_LONG_H_INCLUDED
