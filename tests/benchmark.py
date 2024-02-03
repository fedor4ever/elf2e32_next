# encoding=utf-8
import os, errno, subprocess, timeit

elf2e32_next=r"../bin/Release/elf2e32.exe"

implibs=r""" --libpath="SDK_libs" """
fpu=r""" --fpu=softvfp"""
append=implibs+fpu

# Creates separate DLL for every call
# append+=""" --capability=All-TCB --defoutput="tmp/out.def" --elfinput="libcrypto.dll" --output="tmp/libcrypto-2.4.5(%d).dll" --linkas="libcrypto{000a0000}.dll" --dso="tmp/libcrypto{000a0000}.dso" --targettype=STDDLL --dlldata --namedlookup --excludeunwantedexports --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 """

append+=""" --capability=All-TCB --defoutput="tmp/out.def" --elfinput="libcrypto.dll" --output="tmp/libcrypto-2.4.5.dll" --linkas="libcrypto{000a0000}.dll" --dso="tmp/libcrypto{000a0000}.dso" --targettype=STDDLL --dlldata --namedlookup --excludeunwantedexports --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 """

counter=1

def Bench(args):
   global counter
   tmp=args[0]
   try:
      # subprocess.call(args + append)
# Creates separate DLL for every call
      # subprocess.call(args + append %counter, stdout=open(os.devnull, 'wb'))
      subprocess.call(args + append, stdout=open(os.devnull, 'wb'))
      print "step %d" %counter
   except:
      print ""
      print "Unexpectable test failure:\n %s) %s" %(counter, args + append)
   finally:
      counter+=1

if __name__ == "__main__":
    # execute only if run as a script
   try:
      os.makedirs("tmp")
   except OSError as e:
      if e.errno == errno.EEXIST:
         pass
   import timeit
   t1 = timeit.timeit("Bench('elf2e32')", setup="from __main__ import Bench", number=10)
   t2 = timeit.timeit("Bench(\"%s\")" %elf2e32_next, setup="from __main__ import Bench", number=10)
   
   print "Work time for"
   print "original elf2e32: %s" %t1
   print "elf2e32_next: %s" %t2
   boost = t2 + t1/t2*100
   print "Perfomance boost: %s%%" %boost
    