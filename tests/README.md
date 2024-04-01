## Intro
Good targets used in tests to compare with original elf2e32, broken and bad - excluded.

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

In case if you want to dig yourself - [vbindiff](https://www.cjmweb.net/vbindiff/) can help. WinXP version in tests folder. Don't forget to unpack compressed E32Image =)

**How open bug report**:
 - pack in zip exe\dll, .dso(if any), .def(if any) from project folder in *epoc32/build*, .mmp file
 - open issue request and attach that zip
 - sources - not needed at all

## Testing the elf2e32 build

**Testing your build**:
 - test_all.py - fast, python 2 needed
 - sdk_all_app_builder.py - slow despite multithreading, Symbian SDK needed

If prints "Good Job! All test passed! =D" - all done.
If prints "Unexpectable test failure:" or "But something goes wrong" - oops...

In case test_all.py: run test_common.py, test_dll_options.py, test_dso_options.py, test_exe_options.py and check output.

**Set up sdk_all_app_builder.py**
This test required fully configured Symbian SDK and python 2. Before run 3 variables needed to set: path2sdk, elf2e32_x86, elf2e32_x64. Also elf2e32 should builded with param `-DSET_COMPILETIME_LOAD_EXISTED_FILECRC`. This force to load CRC files for DSO and E32. If load fails - create CRC near DSO and E32.

Thus at first step this script does normal build near 200 examples from SDK. At second step - create CRC near DSO and E32. After that replace elf2e32 and start bulid for x86 and x64 versions.

Sometimes this script crashes and does't restore original elf2e32. In that case in [SDK folder]/epoc32/tools and rename elf2e32_old.exe to elf2e32.exe.

There script `fix_s60v5_sdk.py` - fix build errors with modern compilers in examples from S60_5th_Edition_SDK_v1.0.
