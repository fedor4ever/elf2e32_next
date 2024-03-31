# encoding=utf-8
import os, subprocess

elf2e32=r"..\bin\Release\elf2e32.exe"
failed_tests = 0
failed_tests_data = []

defin=r""" --definput="libcryptou.def" """
dsoout=r""" --dso="tmp\libcrypto{000a0000}.def2dso.dso" """
linkas=r""" --linkas="libcrypto{000a0000}.dll" """
tgttype=r" --targettype=implib"


dsodefTests=(
(elf2e32+defin+dsoout+linkas+tgttype, "Simple def2dso creation.\n Options are: %s\n", ("tmp\libcrypto{000a0000}.def2dso.dso", )),
(elf2e32+""" --elfinput="tmp\libcrypto{000a0000}.def2dso.dso" """ + """ --defoutput="tmp\dso2def.(01).def" """,
"dso2def creation with simplified syntax.\n Options are: %s\n", ("tmp\dso2def.(01).def", )),
(elf2e32+""" --definput="tmp\dso2def.(01).def" """ + """ --dso="tmp\dso2def2dso.(02).dso" """,
"Make dso from def generated from dso.\n Options are: %s\n", ("tmp\dso2def2dso.(02).dso", )),
(elf2e32+defin+ """--dso="tmp\def2dso.(03).dso" """ + """  --linkas="def2dso2def.(03).dll" """ + """ --filecrc=testing_CRCs/def2dso.(03).dcrc """,
"Make dso from def which made from dso with custom --linkas... Options are: %s\n", ("tmp\def2dso.(03).dso", )),
(elf2e32+""" --elfinput="tmp\def2dso.(03).dso" """ + """ --defoutput="tmp\def2dso2def.(04).def" """,
"Make def from dso which made from def...\n Options are: %s\n", ("tmp\def2dso2def.(04).def", )),
)

def SuceededTests(*args):
   """These tests must alwais pass!"""
   global failed_tests
   tmp=args[0]
   str=tmp[1] %tmp[0]
   try:
      print "\n"+tmp[0]
      subprocess.check_call(tmp[0])
      ReportIfMissingOutput(tmp[2], str)
   except:
      print "Unexpectable test failure:\n %s\n" %str
      failed_tests+=1
   else:
      print "Test succeeded: %s!\n" %str

def ReportIfMissingOutput(artifacts, msg):
   global failed_tests_data
   tmp = [x for x in artifacts if not os.path.isfile(x)]
   if len(tmp) == 0:
      return
   tmp = "Test %s doesn't produce build artifacts: %s" %(msg, str(tmp))
   failed_tests_data.append(tmp)

def Run():
   for x in dsodefTests:
      SuceededTests(x)

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