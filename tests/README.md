## Intro
Good and bad targets used in tests, broken - excluded.

**Good targets:**
 - AlternateReaderRecog.dll
   * nothing special
 - cmd_test.exe.elf
   * ordinal exe with explicit exported symbol
 - kf_Python_launcher.exe
   * nothing special
 - libcrypto-2.4.5.SDK.dll
   * STDDLL, may tested only if interface of the library not changed.

**Bad targets:**
 - kf__speedups.pyd
	* has no exports but elf2e32 succesfully produce E32Image
 - basicdll.elf
	* has no exports and crashes elf2e32

**Broken targets:**
 - STDDLL - if interface of the library is changed.
	* Confirmed for elf2e32 at:
	* * Nokia_Symbian_Belle_SDK_v1.0 - runtime crash
	* * S60_5th_Edition_SDK_v1.0 - runtime crash
	* * Some old SDK - exit with error code

## Building targets

**Build option for target**(tuned for Codeblock):
1) *AlternateReaderRecog.dll*:
`--capability=ProtServ --defoutput=tests\tmp\AlternateReaderRecog{000a0000}.def --elfinput="tests\AlternateReaderRecog.dll" --output="tests\tmp\AlternateReaderRecogE32.dll" --libpath=tests\SDK_libs --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tests\tmp\AlternateReaderRecog{000a0000}.dso --fpu=softvfp --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sysdef=_Z24ImplementationGroupProxyRi,1; --sid=0x101ff1ec --version=10.0 --ignorenoncallable --debuggable --smpsafe --uncompressed --filecrc=tests\AlternateReaderRecog.SDK.crc`
2) *cmd_test.exe.elf*:
`--capability=none --elfinput=tests\cmd_test.exe.elf --output=tests\tmp\cmd_teste32.exe --libpath="tests\SDK_libs" --linkas=cmd_test{000a0000}[e8181dba].exe --fpu=softvfp --uid1=0x1000007a --uid2=0x00000000 --uid3=0xe8181dba --sid=0xe8181dba --targettype=EXE --uncompressed --version=10.0 --verbose --filecrc=tests\cmd_test.crc`
3) *kf_Python_launcher.exe*:
`--capability=All-TCB --elfinput="tests\kf_Python_launcher.exe" --output="tests\tmp\kf_Python_launcher.tst.exe" --libpath="tests\SDK_libs"  --linkas="kf_Python_launcher{000a0000}.exe" --fpu=softvfp --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --targettype=STDEXE --dlldata --ignorenoncallable --debuggable --uncompressed --filecrc="tests\kf_Python_launcher.crc"`
4) *libcrypto-2.4.5.SDK.dll*:
`--capability=All-TCB --definput="tests\libcryptou.def" --defoutput="tests\tmp\out.def" --elfinput="tests\libcrypto.dll" --output="tests\tmp\libcrypto-2.4.5.tst.dll" --libpath="tests\SDK_libs" --linkas="libcrypto{000a0000}.dll" --dso="tests\tmp\libcrypto{000a0000}.dso" --fpu=softvfp --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --targettype=STDDLL --dlldata --ignorenoncallable --uncompressed --debuggable --smpsafe --filecrc="tests\libcrypto-2.4.5.SDK.crc"`

## Compatibility
Elf2e32 from different SDK may produce different DSO from this one.
Option to create DSO: --dso="<path><dsoname>". This one and from Nokia_Symbian_Belle_SDK_v1.0 store dsoname only, from S60_3rd_FP2_SDK_v1.1 store full path. Store dsoname only easy to reproduce and compare.
**Build option**:
` --capability=All-TCB --defoutput="tests/tmp/out.(0).def" --elfinput="tests/libcrypto.dll"  --output="tests/tmp/libcrypto-2.4.5.(0).dll" --libpath="tests/SDK_libs" --linkas="libcrypto{000a0000}.dll" --dso="tests/tmp/libcrypto{000a0000}.(0).dso" --fpu=softvfp --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --targettype=STDDLL --dlldata --ignorenoncallable --uncompressed --unfrozen --definput="tests/libcryptou_openssl.def" --smpsafe --debuggable --filecrc="tests/testing_CRCs/dll_outdated.crc;tests/testing_CRCs/dll_outdated.dcrc" --verbose=-1`

## Testing your app

**Perform the following steps**:
 - build a program using the utilities from the SDK
 - create .crc file with mine elf2e32 using command
`elf2e32 --filecrc --e32input=<file>`
 - copy mine *elf2e32* to epoc32\tools, save original
 - build a program using the utilities from the SDK with same flags
 - compare builded program using command
`elf2e32 --filecrc=<crcfile> --e32input=<file>`
 - if doesn't print "All CRC matches!" - please open bug report.

**How open bug report**:
 - pack in zip exe\dll, .dso(if any), .def(if any) from project folder in *epoc32/build*, .mmp file
 - open issue request and attach that zip
 - sources - not needed at all

## Testing the elf2e32 build

**Testing your build**:
 - run test_all.py
 - if print "Good Job! All test passed! =D" - all done.
 - if print "Unexpectable test failure:" - run test_common.py, test_dll_options.py, test_dso_options.py, test_exe_options.py

**Some old stuff, ignore**
 - Verify these files using [vbindiff](https://www.cjmweb.net/vbindiff/). Difference allowed for these ranges:
1. For E32Image
* ***0x0058 - 0x0059***(E32ImageHeader::iExportDirOffset) - Should be zero for EXE without exports
* ***0x0015 - 0x0018***(E32ImageHeader::iHeaderCrc)
* ***0x0021 - 0x0024***(ToolVersion E32ImageHeader::iVersion)
* ***0x0025 - 0x002A***(E32ImageHeader::iTimeLo and E32ImageHeader::iTimeHi)
* ***0x002B - 0x0020***(E32ImageHeader::iFlags)
If there are differences in another ranges, please report the error.
2. For DSO:
First two bytes in .version section with type VERSYM. For check use command arm-none-symbianelf-readelf.exe -S <you_dsofile.dso> or enable macro EXPLORE_DSO_BUILD for detailed output about sections in generated DSO.
