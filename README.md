
# elf2e32_next

Equal with vanilla elf2e32 v. 2.1.15

*elf2e32_next* is dropin replacement for broken elf2e32 in Symbian SDKs.
Supported SDKs: prior Nokia_Symbian_Belle_SDK_v1.0.
Tested SDKs: S60_3rd_FP2_SDK_v1.1, S60_5th_Edition_SDK_v1.0, UIQ3.3SDK_BETA.
SDK help telling about this:

> The post-linker is a Symbian-specific build tool. Its principal
> function is to create executables (DLLs and EXEs) in the particular
> format required by Symbian OS, known as the E32Image format.
> Commercial compiler and linkers, such as ARM's RVCT and GNU GCC,
> cannot directly generate that format. Such compilers do however
> support a standard output format called the Executable and Linking
> Format (ELF) (defined at http://www.caldera.com/developers/gabi/). A
> tool is therefore needed to convert from the ELF format into the
> E32Image format, and this is the key role of the post-linker. The tool
> also contains the functionality to maintain and create the DEF files
> used for freezing the exports of libraries.

## Features:
 - strict validation for E32Image consistency(builded and existed too!)
 - meaningfull report if validation failed
 - easy build for different OS
 - fixed bad E32Image output
 - fixed crash for specific option combination
 - all public cmdline options supported and their arguments from versions prior Belle SDK
 - very strict against illformend cmdline options
 - stand alone mode
 - robust and simple code for binary hackers
 - generate header file with exported functions from DSO file or in post-linker part
 - gets exported functions from DSO and store in DEF file
 - looking for new bad ELF files for testing
 - list global variables if --dlldata not specified for DLL target
 - repacking existing E32 image
 - list global variables if --dlldata not specified for any targets except STDDLL and STDEXE

## Changes:
- option --e32input=<file> works as --e32input=<file> --dump=h, was --e32input=<file> --dump=hscdeit

## You wish to add new enhansements.
If you wish add some new tricks for E32 Image target use new modules:
 - E32File for binary generation
 - E32Parser for get E32 Image layout properties
 - E32Validator for checking E32 Image consistency
 - e32common.h for use E32 Image common structures

## Build instruction
 - Code block users can import and build
 - Other - need C++14 compiler
 - Run tests

## Strict validation
 - Checks for valid params
 - Check if DLL has exported symbols

## Bad E32Image output
### Bad E32Image output
 - if you forget to mark function(s) with EXPORT_C/IMPORT_C original version in worst case it creates something unverified in E32Image format. Symptom - DEF file without exported functions. Without crash! Example: tests/kf__speedups.pyd
Fix - stop job with error message

> elf2e32 --capability=AllFiles+TCB --defoutput="tmp\kf__speedups.def" --elfinput="kf__speedups.pyd" --output="tmp\kf__speedups.tst.pyd" --libpath="SDK_libs" --linkas="kf__speedups{000a0000}.dll" --fpu=softvfp --uid1=0x10000079 --uid2=0x00000000  --uid3=0x00000000 --dso="tmp\libcrypto{000a0000}.dso" --targettype=DLL

### Crash instead E32Image output
 - if you forget to mark function(s) with EXPORT_C/IMPORT_C original version in early SDKs stop working from unknown error, later versions crashes at runtime. It's same as EXPORT_C/IMPORT_C case technically.
Fix - stop job with error message

 - crash at runtime for STDDLL target if exports in DEF frozen and any functions removed in library. Example: examples/stddll_crashes_elf2e32.zip
Fix - warning message and create something unverified in E32Image format. Use at your own risk

### Resume
STDDLL target broken in many ways. There one way it safe - library with fixed and constant interface. Do not use with big libraries with evolving interface. Build DLL target instead with GCC option -fvisibility=default if you don't mark function(s) with EXPORT_C/IMPORT_C.

## Stand alone mode
This version can used directly with other build systems. For that it deduce several cmdline option created by build scripts in SDK. As example SDK passes that build options:
> elf2e32 --uid1=0x10000079 --uid2=0x20004c45 --uid3=0xe9e357a1 --sid=0xe9e357a1 --debuggable --version=10.0 --capability=none --fpu=softvfp --targettype=STDDLL --output=epoc_stddll_example.dll --definput=stddll_example.def --dso=\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\LIB\stddll_example{000a0000}.dso --defoutput=stddll_example{000a0000}.def --elfinput=stddll_example.dll --linkas=stddll_example{000a0000}[e9e357a1].dll --namedlookup --libpath=\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\LIB

and reduced options:
> elf2e32 --uid2=0x20004c45 --uid3=0xe9e357a1 --debuggable --capability=none --targettype=STDDLL --output=epoc_stddll_example.dll --definput=stddll_example.def --defoutput=stddll_example{000a0000}.def --elfinput=stddll_example.dll --namedlookup --dso --libpath=\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\LIB

As you see options for **uid1**, **sid**, **fpu**, **version**, **linkas**, **stack** omitted. Also **sysdef** option maybe omitted too for ECOM plugins. Option **dso** argument omitted. If **uid2** and **uid3** are zero they can omitted too. Rules:
 - uid1 - deduced from targettype
 - sid - sid=uid3
 - fpu - fpu=softvfp
 - version = version=10.0
 - dso - deduced DSO location is first set argument from: elfinput or E32output or defoutput
 - linkas - deduced from elfinput and version and uid3
 - sysdef - deduced for Ecom plugins
 - stack - exe only, default value 8kb

## Repacking existing E32 image
Syntax: `elf2e32 --e32input=<input> --output=<output> --compressionmethod=<compression>`

## Nokia_Symbian_Belle_SDK_v1.0
SDK lacks documentation for elf2e32 syntax. Also new options added to elf2e32 and I have no sources. New options accepted but not processed.

## Version numbering
Elf2e32 stores own version in output E32Image.
It has particular format: major, minor and build. Major and minor stored as uint8_t, build as uint16_t.

Option version value should come in <Major>.<Minor> form. Their parts should fit in uint16_t range.
