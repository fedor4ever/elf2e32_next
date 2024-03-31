# encoding=utf-8
import os, subprocess

# elf2e32=r"elf2e32.exe"
elf2e32=r"..\bin\Release\elf2e32.exe"
implibs=r""" --libpath="SDK_libs" """

counter=0
failed_tests = 0
failed_tests_data = []

elf2e=elf2e32+r" --verbose=-1"
elf2e32+=r" --debuggable --smpsafe --verbose=-1"

# libcrypto
caps=" --capability=All-TCB"
defin=r""" --definput="libcryptou.def" --filecrc=libcrypto-2.4.5.SDK.crc """
defout=r""" --defoutput="tmp\out.(%02d).def" """
elfin=r""" --elfinput="libcrypto.dll" """
e32bin=r""" --output="tmp\libcrypto-2.4.5.(%02d).dll" """
linkas=r""" --linkas="libcrypto{000a0000}.dll" """
dsoout=r""" --dso="tmp\libcrypto{000a0000}.(%02d).dso" """
fpu=r" --fpu=softvfp"
uid1=r" --uid1=0x10000079"
uid2=r" --uid2=0x20004c45"
uid3=r" --uid3=0x00000000"
tgttype=r" --targettype=STDDLL"
tail=r" --dlldata --ignorenoncallable --uncompressed"


# --capability=All-TCB --definput="tests\libcryptou.def" --defoutput="tests\out.def" --elfinput="tests\libcrypto.dll" --output="tests\libcrypto-2.4.5.tst.dll" --libpath="D:\Symbian\S60_5th_Edition_SDK_v1.0\epoc32\release\armv5\lib" --linkas="libcrypto{000a0000}.dll" --dso="tests\tmp\libcrypto{000a0000}.dso" --fpu=softvfp --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --targettype=STDDLL --dlldata --ignorenoncallable --debuggable --smpsafe --uncompressed --filecrc=tests\libcrypto-2.4.5.SDK.crc

longtail=e32bin+implibs+linkas+dsoout+fpu+uid1+uid2+uid3+tgttype+tail

# Build options
areader_cmd = elf2e+implibs+fpu+""" --capability=ProtServ --defoutput=tmp\AR.def --elfinput="AlternateReaderRecog.dll" --output="tmp\AlternateReaderRecogE32.dll" --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tmp\AlternateReaderRecog{000a0000}.dso --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sid=0x101ff1ec --version=10.0 --ignorenoncallable --sysdef=_Z24ImplementationGroupProxyRi,1; --uncompressed   --filecrc=AlternateReaderRecog.SDK.crc;AlternateReaderRecog.SDK.dcrc """

outdated_def_file = elf2e32+caps+defout+elfin+longtail+r" --unfrozen"+r""" --definput="libcryptou_openssl.def" """ + """ --filecrc="libcrypto-2.4.5.SDK.crc;testing_CRCs/dll_outdated.dcrc" """
dso2def = elf2e32+""" --elfinput="libcrypto{000a0000}.dso" """+defout
def2def = elf2e32+defin+ """ --defoutput="tmp\def2def.def" """
elf2baree32 = elf2e32+elfin+""" --output="tmp\elf2baree32.dll" --filecrc=tmp\elf2baree32.dcrc """+implibs+tgttype+linkas+tail

args1=(

("Test #%d: binary creation with outdated def file",
outdated_def_file,
outdated_def_file,
("tmp\out.(%02d).def", "tmp\libcrypto-2.4.5.(%02d).dll", "tmp\libcrypto{000a0000}.(%02d).dso"),
),
("Test #%d: full options list",
elf2e+caps+defin+defout+elfin+longtail,
"Full options list!",
("tmp\out.(%02d).def", "tmp\libcrypto-2.4.5.(%02d).dll", "tmp\libcrypto{000a0000}.(%02d).dso"),
),
("Test #%d: dso2def conversion",
dso2def,
"dso2def conversion!",
("tmp\out.(%02d).def", ),
),
("Test #%d: def2def conversion",
def2def,
"def2def conversion!",
("tmp\def2def.def", ),
),
("Test #%d: simple binary creation",
elf2baree32,
"Simple binary creation!",
("tmp\elf2baree32.dll", ),
),
("Test #%d: full binary creation for ECOM plugin",
areader_cmd,
"Full binary creation for ECOM plugin!",
("tmp\AR.def", "tmp\AlternateReaderRecogE32.dll", "tmp\AlternateReaderRecog{000a0000}.dso", ),
) )

def SuceededTests(*args):
   """These tests must alwais pass!"""
   global counter
   global failed_tests
   tmp=args[0]
   try:
      longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+uid1+uid2+uid3+tgttype+tail
      print tmp[0] %counter
      tmp1=tmp[1].replace("%02d", str(counter))
      print tmp1
      print "\n"
      subprocess.check_call(tmp1)
      t = map(lambda x: x.replace("%02d", str(counter)), tmp[3])
      ReportIfMissingOutput(t, tmp[0] %counter)
   except:
      print "Unexpectable test #%s failure:\n %s" %(counter, tmp[2])
      failed_tests+=1
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


args=(
("Test #%d: elfin without args",
elf2e32+caps+defin+defout+elfin.split("=")[0]+longtail,
elfin.split("=")[0],
),
("Test #%d: defin without args",
elf2e32+caps+defin.split("=")[0]+defout+elfin+longtail,
defin.split("=")[0],
),
("Test #%d: defin and defout without args",
elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin+longtail,
defin.split("=")[0]+defout.split("=")[0],
),
("Test #%d: create all except defoutput",
elf2e32+caps+defin+defout.split("=")[0]+elfin+longtail,
defout.split("=")[0],
),
("Test #%d: e32image creation without args in params defin defout elfin",
elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin.split("=")[0]+longtail,
elf2e32+caps+defin.split("=")[0]+defout.split("=")[0]+elfin.split("=")[0]+longtail,
),
("Test #%d: elf2e32 conversion without other options",
elf2e32+elfin+""" --output="tmp\elf2baree32.dll" """,
elf2e32+elfin+""" --output="tmp\elf2baree32.dll" """,
) )

# try:
   # print  %counter
   # print 
   # subprocess.check_call()
# except:
   # print "Unexpectable test failure: %s" %()
# finally:
   # print "\n"
   # counter+=1

def FailureTests(*arg):
   """These tests must alwais fail!"""
   global counter
   global failed_tests
   tmp=arg[0]
   try:
      longtail=e32bin %counter+implibs+linkas+dsoout %counter+fpu+uid1+uid2+uid3+tgttype+tail
      print tmp[0] %counter
      tmp1=tmp[1].replace("%02d", str(counter))
      print tmp1
      subprocess.check_call(tmp1)
      failed_tests+=1
   except:
      print "Expectable test #%s failure:\n %s" %(counter, tmp[2])
   finally:
      print "\n"
      counter+=1

def Run():
   print "Tests running"
   for x in args1:
      SuceededTests(x)
   for y in args:
      FailureTests(y)

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
