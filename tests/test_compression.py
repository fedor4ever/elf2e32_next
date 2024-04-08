# encoding=utf-8
import os, subprocess

# elf2e32=r"elf2e32.exe"
elf2e32 = os.path.join("..", "bin", "Release", "elf2e32")

counter=0
failed_tests = 0
failed_tests_data = []

opts=(
(''' --filecrc="kf_Python_launcher.crc" --e32input="kf_Python_launcheE32.exe" --output="tmp\kf_Python_none.exe" --compressionmethod=none''',
"tmp\kf_Python_none.exe" ),
(''' --filecrc="kf_Python_launcher.crc" --e32input="kf_Python_launcheE32.exe" --output="tmp\kf_Python_inf.exe" --compressionmethod=inflate''',
"tmp\kf_Python_inf.exe"),
(''' --filecrc="kf_Python_launcher.crc" --e32input="kf_Python_launcheE32.exe" --output="tmp\kf_Python_byt.exe" --compressionmethod=bytepair''',
"tmp\kf_Python_byt.exe" ),

(''' --filecrc="kf_Python_launcher.crc" --e32input="tmp\kf_Python_byt.exe" --output="tmp\kf_Python_byt2inf.exe" --compressionmethod=inflate''',
"tmp\kf_Python_byt2inf.exe"),
(''' --filecrc="kf_Python_launcher.crc" --e32input="tmp\kf_Python_inf.exe" --output="tmp\kf_Python_inf2byt.exe" --compressionmethod=bytepair''',
"tmp\kf_Python_inf2byt.exe"),

(''' --filecrc="kf_Python_launcher.crc" --e32input="tmp\kf_Python_byt.exe" --output="tmp\kf_Python_byt2none.exe" --compressionmethod=none''',
"tmp\kf_Python_byt2none.exe"),
(''' --filecrc="kf_Python_launcher.crc" --e32input="tmp\kf_Python_inf.exe" --output="tmp\kf_Python_inf2none.exe" --compressionmethod=none''',
"tmp\kf_Python_inf2none.exe" ),
(''' --e32input="tmp\kf_Python_inf.exe" --output="tmp\kf_Python_inf2none.exe" --compressionmethod=none''',
"tmp\kf_Python_inf2none.exe" ),
)

def SuceededTests(runner, *args):
   """These tests must alwais pass!"""
   global counter
   global failed_tests
   tmp=args[0]
   cmd=runner+tmp[0]
   try:
      print "%s\n" %cmd
      subprocess.check_call(cmd)
      ReportIfMissingOutput(tmp[1], counter)
   except:
      print "Unexpectable test #%s failure:\n %s" %(counter, tmp[0])
      failed_tests+=1
   finally:
      print "\n"
      counter+=1

def ReportIfMissingOutput(artifact, msg):
   global failed_tests_data
   if os.path.isfile(artifact):
      return
   tmp = "Test %s doesn't produce build artifact: %s" %(msg, artifacts)
   failed_tests_data.append(tmp)

def Run(runner = elf2e32):
   print "Tests running"
   [SuceededTests(runner, x) for x in opts]
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