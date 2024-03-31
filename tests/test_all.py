# encoding=utf-8
import os, subprocess, errno

import test_common, test_dll_options, test_dso_options, test_exe_options, test_dll_torture

if __name__ == "__main__":
   try:
      os.makedirs("tmp")
   except OSError as e:
      if e.errno == errno.EEXIST:
         pass

   failed_tests = 0
   print "all tests running"
   
   a, o = test_common.Run()
   b, p = test_dll_options.Run()
   c, q = test_dso_options.Run()
   d, r = test_exe_options.Run()
   e, s = test_dll_torture.Run()
   
   failed_tests = a+b+c+d+e
   failed_tests_data = o+p+q+r+s

   if failed_tests > 0:
      print "Tests failed: %d" %failed_tests
   else:
      print "Good Job! All test passed! =D"
   if len(failed_tests_data) > 0:
      print "But something goes wrong"
      print failed_tests_data
