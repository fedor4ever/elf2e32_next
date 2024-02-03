# encoding=utf-8
import os, subprocess

elf2e32=r"..\bin\Release\elf2e32.exe"
implibs=r""" --libpath="SDK_libs" """

counter=0
failed_tests = 0

caps=" --capability=All-TCB"
# defin=r""" --definput="libcryptou.def" """
# defout=r""" --defoutput="tmp\out.(%02d).def" """
elfin=r""" --elfinput="kf_Python_launcher.exe" --filecrc="kf_Python_launcher.crc" """
e32bin=r""" --output="tmp\kf_Python_launcher.exe" """
# dsoout=r""" --dso="tmp\libcrypto{000a0000}.(%02d).dso" """
fpu=r" --fpu=softvfp"
uid1=r" --uid1=0x10000079"
uid2=r" --uid2=0x20004c45"
uid3=r" --uid3=0x00000000"
tgttype=r" --targettype=STDEXE"
compress=r" --uncompressed"

append=implibs+fpu+compress
longtail=e32bin+uid1+uid2+uid3+tgttype+append

args1=(
("Test #%d: full binary creation for ECOM plugin",
elf2e32+append+""" --capability=ProtServ --defoutput=tmp\AlternateReaderRecog{000a0000}.def --elfinput="AlternateReaderRecog.dll" --output="tmp\AlternateReaderRecogE32.dll"  --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tmp\AlternateReaderRecog{000a0000}.dso  --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sid=0x101ff1ec --version=10.0 --ignorenoncallable --debuggable --sysdef=_Z24ImplementationGroupProxyRi,1; --filecrc=AlternateReaderRecog.SDK.crc """,
"creation ECOM plugin failed!",
),
("Test #%d: simple exe creation.\n",
elf2e32+caps+elfin+longtail+" --debuggable",
elf2e32+caps+elfin+longtail+" --debuggable",
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
      print "Unexpectable test failure:\n %s" %tmp[2]
   finally:
      print "\n"
      counter+=1

def Run():
   print "Tests running"
   for x in args1:
      SuceededTests(x)

   if failed_tests > 0:
      print "Tests failed: %d" %failed_tests
   else:
      print "Good Job! All test passed! =D"
   return failed_tests

if __name__ == "__main__":
    # execute only if run as a script
   Run()