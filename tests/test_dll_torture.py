# encoding=utf-8
import os, subprocess, hashlib
from itertools import combinations

# These options chande Export bitmap, Exports, Relocations sections in E32image output:
# --unfrozen
# --ignorenoncallable
# --dlldata
# --excludeunwantedexports
# --namedlookup
# --definput

 # 1) unfrozen + ignorenoncallable + dlldata + excludeunwantedexports + namedlookup + definput
 # 2) unfrozen + ignorenoncallable + dlldata + excludeunwantedexports + namedlookup
 # 3) unfrozen + ignorenoncallable + dlldata + excludeunwantedexports + definput
 # 4) unfrozen + ignorenoncallable + dlldata + excludeunwantedexports

 # 5) unfrozen + ignorenoncallable + dlldata + namedlookup + definput
 # 6) unfrozen + ignorenoncallable + dlldata + namedlookup
 # 7) unfrozen + ignorenoncallable + dlldata + definput
 # 8) unfrozen + ignorenoncallable + dlldata

 # 9) unfrozen + ignorenoncallable + excludeunwantedexports + namedlookup + definput
# 10) unfrozen + ignorenoncallable + excludeunwantedexports + namedlookup
# 11) unfrozen + ignorenoncallable + excludeunwantedexports + definput
# 12) unfrozen + ignorenoncallable + excludeunwantedexports

# 13) unfrozen + ignorenoncallable + namedlookup + definput
# 14) unfrozen + ignorenoncallable + namedlookup
# 15) unfrozen + ignorenoncallable + definput
# 16) unfrozen + ignorenoncallable

# 17) unfrozen + dlldata + excludeunwantedexports + namedlookup + definput
# 18) unfrozen + dlldata + excludeunwantedexports + namedlookup
# 19) unfrozen + dlldata + excludeunwantedexports + definput
# 20) unfrozen + dlldata + excludeunwantedexports

# 21) unfrozen + dlldata + namedlookup + definput
# 22) unfrozen + dlldata + namedlookup
# 23) unfrozen + dlldata + definput
# 24) unfrozen + dlldata

# 25) unfrozen + excludeunwantedexports + namedlookup + definput
# 26) unfrozen + excludeunwantedexports + namedlookup
# 27) unfrozen + excludeunwantedexports + definput
# 28) unfrozen + excludeunwantedexports

# 29) unfrozen + namedlookup + definput
# 30) unfrozen + namedlookup
# 31) unfrozen + definput
# 32) unfrozen

# 33) ignorenoncallable + dlldata + excludeunwantedexports + namedlookup + definput
# 34) ignorenoncallable + dlldata + excludeunwantedexports + namedlookup
# 35) ignorenoncallable + dlldata + excludeunwantedexports + definput
# 36) ignorenoncallable + dlldata + excludeunwantedexports

# 37) ignorenoncallable + dlldata + namedlookup + definput
# 38) ignorenoncallable + dlldata + namedlookup
# 39) ignorenoncallable + dlldata + definput
# 40) ignorenoncallable + dlldata

# 41) ignorenoncallable + excludeunwantedexports + namedlookup + definput
# 42) ignorenoncallable + excludeunwantedexports + namedlookup
# 43) ignorenoncallable + excludeunwantedexports + definput
# 44) ignorenoncallable + excludeunwantedexports

# 45) ignorenoncallable + namedlookup + definput
# 46) ignorenoncallable + namedlookup
# 47) ignorenoncallable + definput
# 48) ignorenoncallable

# 49) dlldata + excludeunwantedexports + namedlookup + definput
# 50) dlldata + excludeunwantedexports + namedlookup
# 51) dlldata + excludeunwantedexports + definput
# 52) dlldata + excludeunwantedexports

# 53) dlldata + namedlookup + definput
# 54) dlldata + namedlookup
# 55) dlldata + definput
# 56) dlldata

# 57) excludeunwantedexports + namedlookup + definput
# 58) excludeunwantedexports + namedlookup
# 59) excludeunwantedexports + definput
# 60) excludeunwantedexports

# 61) namedlookup + definput
# 62) namedlookup
# 63) definput


# ++++++++++++++++++++++++++++++++++++
# CRC naming: UIDENDi means "Unfrozen + Ignorenoncallable + Dlldata + Excludeunwantedexports + Namedlookup + Definput" and so on
# ++++++++++++++++++++++++++++++++++++

# Notes: "--dlldata" needed only if DLL has WSD aka Writable Static Data. But what happens if opt used and DLL hasn't WSD?



elf2e32_SDK=r"elf2e32_belle.exe"
elf2e32_test=r"..\bin\Release\elf2e32.exe"
common_opts=r""" --libpath="SDK_libs" --fpu=softvfp """


libcrypto_defin='--definput="libcryptou.def"'

libcrypto_opts=r""" --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --capability=All-TCB --targettype=STDDLL --elfinput="libcrypto.dll" --linkas="libcrypto{000a0000}.dll" --defoutput="tmp\out_(%02d)_TGT.def" --output="tmp\out_(%02d)_TGT.dll" --dso="tmp\libcrypto{000a0000}.dso" """
libcrypto_opts+=common_opts
libcrypto_opts+=r""" --dlldata """


areader_defin='--definput="AlternateReaderRecog{000a0000}.def"'

areader_opts=r""" --capability=ProtServ --defoutput=tmp\AR_(%02d)_TGT.def --elfinput="AlternateReaderRecog.dll" --output="tmp\AR_(%02d)_TGT.dll" --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tmp\AlternateReaderRecog{000a0000}.dso --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sid=0x101ff1ec --version=10.0 --ignorenoncallable --sysdef=_Z24ImplementationGroupProxyRi,1;lala,2; """
areader_opts+=common_opts



# Torture opts
unfrozen=r""" --unfrozen """
ignorenoncallable=r""" --ignorenoncallable """
dlldata=r""" --dlldata """
excludeunwantedexports=r""" --excludeunwantedexports """
namedlookup=r""" --namedlookup """
definput=libcrypto_defin


def OptSuffix(cmdline):
    tmp = ""
    if("--unfrozen") in cmdline:
        tmp+="U"
    if("--ignorenoncallable") in cmdline:
        tmp+="I"
    if("--dlldata") in cmdline:
        tmp+="D"
    if("--excludeunwantedexports") in cmdline:
        tmp+="E"
    if("--namedlookup") in cmdline:
        tmp+="N"
    if("--definput") in cmdline:
        tmp+="Di"
    return tmp

def CrashMe(sfx):
    if sfx in ("NDi", "UNDi", "INDi", "ENDi", "UINDi", "UENDi", "IENDi", "UIENDi"):
        return True
    return False

#valid error: elf2e32 : Error: E1036: Symbol XXX Missing from ELF File
def SkipMe(sfx):
    if sfx in ('I', 'D', 'E', 'N', 'ID', 'IE', 'IN', 'DE', 'DN', 'EN', 'IDE', 'IDN', 'IEN', 'DEN', 'IDEN'):
        return True
    return False

failed_tests = 0
failed_sfx = []

def BuildE32(elf2e32):
    global failed_tests
    global failed_sfx
    cmdline = elf2e32 + " " + libcrypto_opts
    idx = 1
    args = ['--unfrozen', '--ignorenoncallable', '--excludeunwantedexports', '--namedlookup', libcrypto_defin]
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            tmp = cmdline.replace("%02d", str(idx))
            sfx = OptSuffix(addend)
            tmp=tmp.replace("TGT", sfx)
            print(str(idx) + ') ' + tmp + ' ' + addend)

            old = os.path.join("tmp","libcrypto{000a0000}.dso")
            new = os.path.join("tmp","out_(%02d)_TGT.dso")
            new = new.replace("%02d", str(idx))
            new = new.replace("TGT", sfx)
            if CrashMe(sfx):
               idx+=1
               continue
            try:
                subprocess.check_call(tmp + ' ' + addend)
                os.rename(old, new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --dso=" + new)
                root, ext = os.path.splitext(new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --e32input=" + root + ".dll")
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp + ' ' + addend)
                failed_tests+=1                
                try:
                    os.remove(old)
                except: pass
            finally:
                print "\n"
                idx+=1

    # cmdline = elf2e32 + " " + areader_opts
    # args = ['--unfrozen', '--ignorenoncallable', '--dlldata', '--excludeunwantedexports', '--namedlookup', areader_defin]
    # for x in range(1, len(args)+1):
        # for i in combinations(args, x):
            # addend = ' '.join(i)
            # tmp = cmdline.replace("%02d", str(idx))
            # sfx = OptSuffix(addend)
            # tmp=tmp.replace("TGT", sfx)
            # print(str(idx) + ') ' + tmp + ' ' + addend)

            # old = os.path.join("tmp","AlternateReaderRecog{000a0000}.dso")
            # new = os.path.join("tmp","AR_(%02d)_TGT.dso")
            # new = new.replace("%02d", str(idx))
            # new = new.replace("TGT", sfx)
            # if SkipMe(sfx):
               # continue
            # try:
                # subprocess.check_call(tmp + ' ' + addend)
                # if os.path.isfile(new):
                    # os.remove(new)
                # os.rename(old, new)
                # subprocess.check_call(elf2e32_test + ' --filecrc ' + " --dso=" + new)
                # root, ext = os.path.splitext(new)
                # subprocess.check_call(elf2e32_test + ' --filecrc ' + " --e32input=" + root + ".dll")
            # except:
                # print "Unexpectable test #%s failure:\n %s" %(idx, tmp + ' ' + addend)
                # failed_tests+=1
                # failed_sfx.append(sfx)
                # if os.path.isfile(old):
                    # os.remove(old)
            # finally:
                # print "\n"
                # idx+=1

def AReaderSysdefDCRC(sfx):
    if sfx in ['U', 'UID', 'UIE', 'UIN', 'UDE', 'UDN', 'UEN', 'UIDE', 'UIDN', 'UI', 'UIEN', 'UDEN', 'UD', 'UIDEN', 'UE', 'UN']:
        return os.path.join("testing_CRCs","ar_missing_sysdef.dcrc")
    if sfx in ['EDi', 'NDi', 'UIDi', 'UDDi', 'Di', 'UEDi', 'UNDi', 'IDDi', 'IEDi', 'INDi', 'DEDi', 'DNDi', 'ENDi', 'UIDDi', 'UIEDi', 'UINDi', 'UDEDi', 'UDNDi', 'UENDi', 'IDEDi', 'IDNDi', 'IENDi', 'DENDi', 'UIDEDi', 'UIDNDi', 'UIENDi', 'UDENDi', 'IDENDi', 'UIDENDi', 'UDi', 'IDi', 'DDi']:
        return "AlternateReaderRecog.SDK.dcrc"
    raise "Unknown DCRC' SFX: %s!" %sfx

def AReaderSysdefCRC(sfx):
    if sfx in ['U', 'Di', 'UI', 'UD', 'EDi', 'UID', 'UIE', 'UIDi', 'UDE', 'UDDi', 'UEDi', 'IDDi', 'IEDi', 'DEDi', 'UE', 'UDi', 'IDi', 'DDi', 'UIDE', 'UIDDi', 'UIEDi', 'UDEDi', 'IDEDi', 'UIDEDi']:
        return "AlternateReaderRecog.SDK.crc"
    if sfx in ['NDi', 'UIN', 'UDN', 'UEN', 'UNDi', 'INDi', 'UN', 'DNDi', 'ENDi', 'UIDN', 'UIEN', 'UINDi', 'UDEN', 'UDNDi', 'UENDi', 'IDNDi', 'IENDi', 'DENDi', 'UIDEN', 'UIDNDi', 'UIENDi', 'UDENDi', 'IDENDi', 'UIDENDi']:
        return os.path.join("testing_CRCs","ar_symlook.crc")
    raise "Unknown CRC' SFX: %s!" %sfx

def libcryptoDCRC(sfx):
    if sfx in ['U', 'IE', 'IN', 'EN', 'UIE', 'UIN', 'UEN', 'I', 'IEN', 'UIEN', 'E', 'N', 'UI', 'UE', 'UN']:
        return os.path.join("testing_CRCs","libcrypto_unfrozen.dcrc")
    if sfx in ['IDi', 'EDi', 'UIDi', 'UEDi', 'IEDi', 'UIEDi', 'Di', 'UDi']:
        return "libcrypto{000a0000}.dcrc"
    raise "Unknown DCRC' SFX: %s!" %sfx

def libcryptoCRC(sfx):
    if sfx in ['U', 'I', 'E', 'UI', 'UE', 'IE', 'UIE']:
        return os.path.join("testing_CRCs","libcrypto_unfrozen.crc")
    if sfx in ['Di', 'UDi', 'IDi', 'EDi', 'UIDi', 'UEDi', 'IEDi', 'UIEDi']:
        return "libcrypto-2.4.5.SDK.crc"
    if sfx in ['N', 'UN', 'IN', 'EN', 'UIN', 'UEN', 'IEN', 'UIEN']:
        return os.path.join("testing_CRCs","libcrypto_namedlookup.crc")
    raise "Unknown CRC' SFX: %s!" %sfx

def FindCRCDups(fname, dict):
    with open(fname, 'r') as f:
        key = hashlib.md5(f.read()).hexdigest()
        if key not in dict:
            dict[key] = []
        dict[key].append([fname])

def PrintCRCDups():
    crc = {}
    dcrc = {}
    crcs = []
    dcrcs = []
    tmp = [os.path.join("tmp", file) for file in os.listdir("tmp")]
    tmp1 = [file for file in tmp if file.endswith(".crc")]
    tmp2 = [file for file in tmp if file.endswith(".dcrc")]
    [FindCRCDups(x, crc) for x in tmp1]
    [FindCRCDups(x, dcrc) for x in tmp2]

    for key, val in dcrc.iteritems():
        dcrcs.append(key)
        for x in val:
            tmp = x[0]
            tmp = tmp.split('_')[-1]
            tmp = tmp.split('.')[0]
            dcrcs.append(tmp)

    for key, val in crc.iteritems():
        crcs.append(key)
        for x in val:
            tmp = x[0]
            tmp = tmp.split('_')[-1]
            tmp = tmp.split('.')[0]
            crcs.append(tmp)
    print dcrcs
    print "\n"
    print crcs

def DeduceCRCS():
    BuildE32(elf2e32_SDK)
    PrintCRCDups()

    if failed_tests > 0:
       print "Tests failed: %d\n" %failed_tests
       print failed_sfx
    else:
       print "Good Job! All DLL torture test passed! =D"
    return failed_tests

def BuildAndValidate(): pass

def Run():
    # DeduceCRCS()
    BuildAndValidate()

if __name__ == "__main__":
    # execute only if run as a script
    Run()
