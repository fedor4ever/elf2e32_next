# Copyright (C) 2023 Stryzhniou Fiodar

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


from __future__ import with_statement
import os, subprocess, Queue, threading, errno, time
import multiprocessing as mp
from datetime import datetime

import shutil

import sys
reload(sys)  
sys.setdefaultencoding('utf-8')

path2sdk = "G:/Symbian/S60_5th_Edition_SDK_v1.0/epoc32"
elf2e32_rel = "F:\\codeblock\\elf2e32_next\\bin\\Release\\elf2e32.exe --filecrc --e32input="

app_builder_err_log = "app_builder_err.log"
app_builder_out_log = "app_builder_out.log"
app_builder_time_log = "app_builder_time_log"

# Workaround for "threading bug in strptime"
# See - https://stackoverflow.com/questions/32245560/module-object-has-no-attribute-strptime-with-several-threads-python/46401422
import _strptime


def append2file(path, data):
   save2file(path, data, mode = 'a')

def save2file(path, data, mode = 'w'):
   """Save list elements as strings. Save strings as is"""
   with open(path, mode) as f:
      if type(data) is list:
         for s in data:
            f.write(s + '\n')
      else:
         f.write(data)

def logs(out, err, time_start, time_end, opname):
      start_dt = datetime.strptime(time_start, '%H:%M:%S')
      end_dt = datetime.strptime(time_end, '%H:%M:%S')
      diff = (end_dt - start_dt)
      # I hope it correctly stores logs in parallel tasks.
      # After cmd.communicate() we have ugly 'crcrlf' line endings.
      append2file(app_builder_err_log, out.replace(u"\r", u""))
      append2file(app_builder_out_log, err.replace(u"\r", u""))
      append2file(app_builder_time_log, "Target %s build time: %s.\n" %(opname, str(diff)) )
      print "Target %s done!" %opname


def thread_func(q, plats):
   while True:
      pth = q.get()
      if pth is None:  # EOF?
         return

      cmd = subprocess.Popen('bldmake bldfiles', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=pth, shell=True)
      out, err = cmd.communicate()
      # Clean build directory from previous build.
      cmd = subprocess.Popen('abld reallyclean gcce urel', cwd=pth, shell=True)
      cmd.communicate()

      # Needed because datetime.now() returns the same time for every call.
      start = time.strftime("%H:%M:%S")
      cmd1 = subprocess.Popen('abld build gcce urel', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=pth, shell=True)
      out1, err1 = cmd1.communicate()

      end = time.strftime("%H:%M:%S" )
      start_dt = datetime.strptime(start, '%H:%M:%S')
      end_dt = datetime.strptime(end, '%H:%M:%S')
      diff = (end_dt - start_dt)

      out = out + out1
      out = out.decode('utf-8', errors='ignore')
      err = err + err1
      err = err.decode('utf-8', errors='ignore')
      # I hope it correctly stores logs in parallel tasks.
      # After cmd.communicate() we have ugly 'crcrlf' line endings.
      append2file(app_builder_err_log, out.replace(u"\r", u""))
      append2file(app_builder_out_log, err.replace(u"\r", u""))
      append2file(app_builder_time_log, "Target %s build time: %s.\n" %(pth, str(diff)) )
      print "Target %s done!" %pth

def thread_func1(q, plats):
   while True:
      pth = q.get()
      if pth is None:  # EOF?
         return

      cmd = subprocess.Popen('bldmake bldfiles', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=pth, shell=True)
      out, err = cmd.communicate()

      # Needed because datetime.now() returns the same time for every call.
      start = time.strftime("%H:%M:%S")
      cmd1 = subprocess.Popen('abld build gcce urel', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=pth, shell=True)
      out1, err1 = cmd1.communicate()

      end = time.strftime("%H:%M:%S" )
      start_dt = datetime.strptime(start, '%H:%M:%S')
      end_dt = datetime.strptime(end, '%H:%M:%S')
      diff = (end_dt - start_dt)

      out = out + out1
      out = out.decode('utf-8', errors='ignore')
      err = err + err1
      err = err.decode('utf-8', errors='ignore')
      # I hope it correctly stores logs in parallel tasks.
      # After cmd.communicate() we have ugly 'crcrlf' line endings.
      append2file(app_builder_err_log, out.replace(u"\r", u""))
      append2file(app_builder_out_log, err.replace(u"\r", u""))
      append2file(app_builder_time_log, "Target %s build time: %s.\n" %(pth, str(diff)) )
      print "Target %s done!" %pth

def check_elf2e32():
    t = os.path.join(path2sdk, "tools")
    t = os.path.join(t, "elf2e32")

    cmd = subprocess.Popen(t, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = cmd.communicate()
    
    token = out
    if not token: #elf2e32 2.x can print help in err
        token = err

    if not token:
        raise NameError('No output for elf2e32!')

    tmp = token.splitlines()[1]
    tmp = tmp.split(',')[1]
    if "V2" in tmp:
        print "elf2e32 V2 found"
        return 2
    tmp = tmp.strip()
    if "3." in tmp:
        print "elf2e32 V3 found"
        return 3

def find_projects():
    # Needed because datetime.now() returns the same time for every call.
    start = time.strftime("%H:%M:%S")

    tmp = []
    pth = path2sdk.rsplit('/', 1)[0]
    print pth
    count = 0
    for root, dirs, files in os.walk(pth, topdown=False):
        for name in files:
            if name.endswith("bld.inf"):
                t = os.path.join(root, name)
                # print(t)
                tmp.append(os.path.dirname(t))
                count += 1
            if name.endswith("Bld.inf"):
                t = os.path.join(root, name)
                # print(t)
                tmp.append(os.path.dirname(t))
                count += 1
            if name.endswith("BLD.INF"):
                t = os.path.join(root, name)
                # print(t)
                tmp.append(os.path.dirname(t))
                count += 1

    print "Projects found: %d" %count
    # print tmp
    # save2file("log.txt", tmp)

    end = time.strftime("%H:%M:%S" )
    logs("", "", start, end, "find_projects")
    return tmp

def threads_runner(files, threadfunc):
   q = Queue.Queue()
   for fileName in files:
      q.put(fileName)

   t_count = mp.cpu_count() + 2
   if t_count > q.qsize():
      t_count = q.qsize()

   print "Queue size: %s" %q.qsize()
   print "Thread count: %s" %t_count
   threads = [threading.Thread(target=threadfunc, args=(q, "")) for i in range(t_count)]
   for thread in threads:
      thread.start()
      q.put(None)  # One EOF marker for each thread.
   for thread in threads:
      thread.join()

def build_sdk_elf2e32(sdk_prj):
    if check_elf2e32() > 2:
        raise NameError('Found 3.x version of elf2e32. 2.x expected!')
    threads_runner(sdk_prj, thread_func)


def build_new_elf2e32(sdk_prj):
    t = os.path.join(path2sdk, "tools")
    
    tgt = os.path.join(t, "elf2e32.exe")
    dst = os.path.join(t, "elf2e32_old.exe")
    os.rename(tgt, dst)

    print elf2e32_rel
    rel = elf2e32_rel.split()[0]
    print rel
    shutil.copy(rel, t)
    if check_elf2e32() == 2:
        revert_elf2e32(tgt, dst)
        raise NameError('Found 2.x version of elf2e32. 3.x expected!')
    threads_runner(sdk_prj, thread_func1)
    revert_elf2e32(tgt, dst)

def revert_elf2e32(new, orig):
    os.remove(new)
    os.rename(orig, new)

def make_crc(path):
    tmp = [os.path.join(path, file) for file in os.listdir(path)]
    tmp1 = [file for file in tmp if file.endswith(".dll")]
    tmp2 = [file for file in tmp if file.endswith(".exe")]
    tmp = tmp1 + tmp2

    # Needed because datetime.now() returns the same time for every call.
    start = time.strftime("%H:%M:%S")

    for file in tmp:
        elf2e32_r = elf2e32_rel + file
        cmd = subprocess.Popen(elf2e32_r, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = cmd.communicate()
        os.remove(file) #at next step only only E32image created

    end = time.strftime("%H:%M:%S" )
    logs("", "", start, end, "generate_crc at " + path)
    

def generate_crc():
    build = os.path.join(path2sdk, "release")
    build = os.path.join(build, "gcce")
    build_urel = os.path.join(build, "urel")
    build_udeb = os.path.join(build, "udeb")
    make_crc(build_urel)
    make_crc(build_udeb)

def sdk_all_app_builder():
    start = time.strftime("%H:%M:%S")
    save2file(app_builder_err_log, "")
    save2file(app_builder_out_log, "")
    save2file(app_builder_time_log, "elf2e32 testing started: %s.\n" %start)
    sdk_prj = find_projects()

    build_sdk_elf2e32(sdk_prj)
    generate_crc()
    build_new_elf2e32(sdk_prj)
    # build_new_elf2e32("")

    end = time.strftime("%H:%M:%S" )
    save2file(app_builder_time_log, "elf2e32 testing ended: %s.\n" %end)
    
    start_dt = datetime.strptime(start, '%H:%M:%S')
    end_dt = datetime.strptime(end, '%H:%M:%S')
    diff = (end_dt - start_dt)
    append2file(app_builder_time_log, "elf2e32 testing time: %s.\n" %str(diff) )
    

if __name__ == "__main__":
    sdk_all_app_builder()
