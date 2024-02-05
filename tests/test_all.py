# encoding=utf-8
import os, subprocess

import test_common, test_dll_options, test_dso_options, test_exe_options

if __name__ == "__main__":
   try:
      os.makedirs("tmp")
   except OSError as e:
      if e.errno == errno.EEXIST:
         pass

   failed_tests = 0
   print "all tests running"
   
   failed_tests = test_common.Run()
   failed_tests += test_dll_options.Run()
   failed_tests += test_dso_options.Run()
   failed_tests += test_exe_options.Run()

   if failed_tests > 0:
      print "Tests failed: %d" %failed_tests
   else:
      print "Good Job! All test passed! =D"
