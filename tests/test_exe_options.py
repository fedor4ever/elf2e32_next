# encoding=utf-8
import os, subprocess

elf2e32=r"..\bin\Release\elf2e32.exe"
implibs=r""" --libpath="SDK_libs" """

counter=0
failed_tests = 0

caps=" --capability=All-TCB"
# defin=r""" --definput="libcryptou.def" """
# defout=r""" --defoutput="tmp\out.(%02d).def" """
elfin=r""" --elfinput="kf_Python_launcher.exe" --filecrc=kf_Python_launcher.crc """
e32bin=r""" --output="tmp\kf_Python_launcher.exe" """
linkas=r""" --linkas="kf_Python_launcher{000a0000}.exe" """
# dsoout=r""" --dso="tmp\libcrypto{000a0000}.(%02d).dso" """
fpu=r" --fpu=softvfp"
uid1=r" --uid1=0x10000079"
uid2=r" --uid2=0x20004c45"
uid3=r" --uid3=0x00000000"
tgttype=r" --targettype=STDEXE"
tail=r" --dlldata --ignorenoncallable --debuggable --uncompressed"

longtail=e32bin+implibs+linkas+fpu+uid1+uid2+uid3+tgttype+tail


args=(
("Test #%d: Simple exe creation.",
elf2e32+caps+elfin+longtail,
"Simple exe creation failed!",
),
("Test #%d: exe creation with implicit export _ZdlPvj.",
elf2e32+implibs+fpu+implibs+r""" --sid=0xe8181dba --version=10.0 --uid1=0x1000007a --uid2=0x00000000 --uid3=0xe8181dba --capability=none --targettype=EXE --output=tmp\cmd_teste32.exe --elfinput=cmd_test.exe.elf --linkas=cmd_test{000a0000}[e8181dba].exe --verbose --uncompressed --filecrc=cmd_test.crc """,
"exe creation with implicit export _ZdlPvj failed!",
) )

def SuceededTests(*args):
   """These tests must alwais pass!"""
   global counter
   global failed_tests
   tmp=args[0]
   try:
      print tmp[0] %counter
      print tmp[1]
      subprocess.check_call(tmp[1])
   except:
      print "Unexpectable test elf->exe failure:\n %s" %tmp[2]
      failed_tests+=1
   finally:
      print "\n"
      counter+=1

def Run():
   print "exe tests running"
   for x in args:
      SuceededTests(x)

   if failed_tests > 0:
      print "Tests failed: %d" %failed_tests
   else:
      print "Good Job! All test passed! =D"
   return failed_tests

if __name__ == "__main__":
   Run()
