# encoding=utf-8
import os, subprocess

elf2e32 = os.path.join("..", "bin", "Release", "elf2e32")
implibs=r""" --libpath="SDK_libs" """

counter=0
failed_tests = 0
failed_tests_data = []

caps=" --capability=All-TCB"
# defin=r""" --definput="libcryptou.def" """
# defout=r""" --defoutput="tmp\out.(%02d).def" """
elfin=r""" --elfinput="kf_Python_launcher.exe" --filecrc="kf_Python_launcher.crc" """
e32bin=r""" --output="tmp\kf_Python_launcher_c.exe" """
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
append+""" --capability=ProtServ --defoutput=tmp\AlternateReaderRecog{000a0000}.def --elfinput="AlternateReaderRecog.dll" --output="tmp\AlternateReaderRecogE32_c.dll" --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tmp\AlternateReaderRecog{000a0000}.dso --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sid=0x101ff1ec --version=10.0 --ignorenoncallable --debuggable --sysdef=_Z24ImplementationGroupProxyRi,1; --filecrc=AlternateReaderRecog.SDK.crc;AlternateReaderRecog.SDK.dcrc """,
"creation ECOM plugin failed!",
("tmp\AlternateReaderRecog{000a0000}.def", "tmp\AlternateReaderRecogE32_c.dll", "tmp\AlternateReaderRecog{000a0000}.dso", ),
),
("Test #%d: simple exe creation.\n",
caps+elfin+longtail+" --debuggable",
"simple exe creation failed!",
("tmp\kf_Python_launcher_c.exe", ),
) )


def SuceededTests(runner, *args):
   """These tests must alwais pass!"""
   global counter
   tmp=args[0]
   cmd=runner+tmp[1]
   try:
      print tmp[0] %counter
      print cmd
      subprocess.check_call(cmd)
      ReportIfMissingOutput(tmp[3], tmp[0] %counter)
   except:
      print "Unexpectable test failure:\n %s" %tmp[2]
   finally:
      print "\n"
      counter+=1

def ReportIfMissingOutput(artifacts, msg):
   global failed_tests_data
   tmp = [x for x in artifacts if not os.path.isfile(x)]
   if len(tmp) == 0:
      return
   tmp = "Test %s doesn't produce build artifacts: %s" %(msg, str(tmp))
   failed_tests_data.append(tmp)

def Run(runner = elf2e32):
   print "Tests running"
   for x in args1:
      SuceededTests(runner, x)

   if failed_tests > 0:
      print "Tests failed: %d" %failed_tests
   else:
      print "Good Job! All test passed! =D"
   if len(failed_tests_data) > 0:
      print "But something goes wrong"
      print failed_tests_data
   return failed_tests, failed_tests_data

if __name__ == "__main__":
    # execute only if run as a script
   Run()