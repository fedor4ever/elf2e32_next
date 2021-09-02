# encoding=utf-8
import os, subprocess

elf2e32=r"D:\codeblock\elf2e32_next\bin\Debug\elf2e32.exe"
tstdir=r"D:\codeblock\elf2e32_next\tests"
counter=0

caps=" --capability=All-TCB"
# defin=r""" --definput="libcryptou.def" """
# defout=r""" --defoutput="tmp\out.(%02d).def" """
elfin=r""" --elfinput="kf_Python_launcher.exe" """
e32bin=r""" --output="tmp\kf_Python_launcher.exe" """
implibs=r""" --libpath="\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\lib" """
linkas=r""" --linkas="kf_Python_launcher{000a0000}.dll" """
# dsoout=r""" --dso="tmp\libcrypto{000a0000}.(%02d).dso" """
fpu=r" --fpu=softvfp"
iud1=r" --uid1=0x10000079"
uid2=r" --uid2=0x20004c45"
uid3=r" --uid3=0x00000000"
tgttype=r" --targettype=STDEXE"
tail=r" --dlldata --ignorenoncallable --debuggable --smpsafe --uncompressed"

longtail=e32bin+implibs+linkas+fpu+iud1+uid2+uid3+tgttype+tail


args=(
("Test #%d: Simple exe creation.",
elf2e32+caps+elfin+longtail,
"Simple exe creation failed!",
),
("Test #%d: exe creation with implicit export _ZdlPvj.",
elf2e32+implibs+fpu+r""" --sid=0xe8181dba --version=10.0 --uid1=0x1000007a --uid2=0x00000000 --uid3=0xe8181dba --capability=none --targettype=EXE --output=tmp\cmd_teste32.exe --elfinput=cmd_test.exe.elf --linkas=cmd_test{000a0000}[e8181dba].exe --libpath=\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\LIB --verbose  --uncompressed""",
"exe creation with implicit export _ZdlPvj failed!",
) )

def SuceededTests(*args):
   """These tests must alwais pass!"""
   global counter
   tmp=args[0]
   try:
      print tmp[0] %counter
      print tmp[1]
      subprocess.check_call(tmp[1])
   except:
      print "Unexpectable test elf->exe failure:\n %s" %tmp[2]
   finally:
      print "\n"
      counter+=1

if __name__ == "__main__":
   print "exe tests running"
   for x in args:
      SuceededTests(x)
