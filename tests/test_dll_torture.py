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



# For BELLE version increased default value for heap max if not provided option --heap.
# Raised max heap size up to 0x200000.
# For compatibility with previous versions used option --heap=0x1000,0x100000
common_opts=r""" --libpath="SDK_libs" --fpu=softvfp --heap=0x1000,0x100000 """


elf2e32_SDK=r"elf2e32_belle.exe"
elf2e32_test=r"..\bin\Release\elf2e32.exe"


libcrypto_defin='--definput="libcryptou.def"'

libcrypto_opts=r""" --uid1=0x10000079 --uid2=0x20004c45 --uid3=0x00000000 --capability=All-TCB --targettype=STDDLL --elfinput="libcrypto.dll" --linkas="libcrypto{000a0000}.dll" --defoutput="tmp\out_(%02d)_TGT.def" --output="tmp\out_(%02d)_TGT.dll" --dso="tmp\libcrypto{000a0000}.dso" """
libcrypto_opts+=common_opts
libcrypto_opts+=r""" --dlldata """


areader_defin='--definput="AlternateReaderRecog{000a0000}.def"'

areader_opts=r""" --capability=ProtServ --defoutput=tmp\AR_(%02d)_TGT.def  --elfinput="AlternateReaderRecog.dll" --output="tmp\AR_(%02d)_TGT.dll" --linkas=AlternateReaderRecog{000a0000}[101ff1ec].dll --dso=tmp\AlternateReaderRecog{000a0000}.dso --uid1=0x10000079 --uid2=0x10009d8d --uid3=0x101ff1ec --targettype=PLUGIN --sid=0x101ff1ec --version=10.0 """
areader_normal_opts = areader_opts + common_opts + r"""  --sysdef=_Z24ImplementationGroupProxyRi,1;lala,2; """



# Torture opts
unfrozen=r""" --unfrozen """
ignorenoncallable=r""" --ignorenoncallable """
dlldata=r""" --dlldata """
excludeunwantedexports=r""" --excludeunwantedexports """
namedlookup=r""" --namedlookup """
definput=libcrypto_defin


def save2file(path, data, mode = 'w'):
    """Save list elements as strings. Save strings as is"""
    with open(path, mode) as f:
        if type(data) is list:
            for s in data:
                f.write(s + '\n')
        else:
            f.write(data)

def append2file(path, data):
    save2file(path, data, mode = 'a')

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
def SkipMe(addend):
    sfx = OptSuffix(addend)
    if sfx in ('I', 'D', 'E', 'N', 'ID', 'IE', 'IN', 'DE', 'DN', 'EN', 'IDE', 'IDN', 'IEN', 'DEN', 'IDEN'):
        return True
    return False

failed_tests = 0
started_tests = 0
failed_sfx = []

def NewTgt(tmplate, idx, sfx):
    new = os.path.join("tmp", tmplate)
    new = new.replace("%02d", str(idx))
    new = new.replace("TGT", sfx)
    return new

def InitTest(addend, cmdline, idx, old_dso, new_dso, callback = None):
    global started_tests
    started_tests+=1
    tmp = cmdline.replace("%02d", str(idx))
    sfx = OptSuffix(addend)
    tmp=tmp.replace("TGT", sfx)
    if callback:
        tmp+=callback(sfx)
    tmp = tmp + ' ' + addend
    print(str(idx) + ') ' + tmp + ' ' + addend)

    old = os.path.join("tmp",old_dso)
    new = NewTgt(new_dso, idx, sfx)
    return tmp, sfx, old, new

def E32WithFrozenDEF(elf2e32):
    global failed_tests
    global failed_sfx
    cmdline = elf2e32 + " --uncompressed " + libcrypto_opts
    idx = 1
    args = ['--unfrozen', '--ignorenoncallable', '--excludeunwantedexports', '--namedlookup', libcrypto_defin]
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            tmp, sfx, old, new = InitTest(addend, cmdline, idx, "libcrypto{000a0000}.dso", "out_(%02d)_TGT.dso")
            if CrashMe(sfx):
               idx+=1
               continue
            try:
                subprocess.check_call(tmp)
                os.rename(old, new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --dso=" + new)
                root, ext = os.path.splitext(new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --e32input=" + root + ".dll")
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp)
                failed_tests+=1                
                try:
                    os.remove(old)
                except: pass
            finally:
                print "\n"
                idx+=1

def TortureECOM(elf2e32):
    global failed_tests
    global failed_sfx
    cmdline = elf2e32 + " " + areader_opts + common_opts
    idx = 1
    args = ['--unfrozen', '--ignorenoncallable', '--dlldata', '--excludeunwantedexports', '--namedlookup', areader_defin]
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            if SkipMe(addend):
               continue
            tmp, sfx, old, new = InitTest(addend, cmdline, idx, "AlternateReaderRecog{000a0000}.dso", "AR_(%02d)_TGT.dso")
            # tmp += " --uncompressed"

            try:
                subprocess.check_call(tmp)
                if os.path.isfile(new):
                    os.remove(new)
                os.rename(old, new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --dso=" + new)
                root, ext = os.path.splitext(new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --e32input=" + root + ".dll")
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp)
                failed_tests+=1
                failed_sfx.append(sfx)
                if os.path.isfile(old):
                    os.remove(old)
            finally:
                print "\n"
                idx+=1

def ValidateECOM(elf2e32):
    global failed_tests
    global failed_sfx
    cmdline = elf2e32 + " " + areader_opts + common_opts
    idx = 1
    args = ['--unfrozen', '--ignorenoncallable', '--dlldata', '--excludeunwantedexports', '--namedlookup', areader_defin]
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            tmp, sfx, old, new = InitTest(addend, cmdline, idx, "AlternateReaderRecog{000a0000}.dso", "AR_(%02d)_TGT.dso")
            tmp += " --uncompressed"
            try:
                subprocess.check_call(tmp)
                if os.path.isfile(new):
                    os.remove(new)
                os.rename(old, new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --dso=" + new)
                root, ext = os.path.splitext(new)
                subprocess.check_call(elf2e32_test + ' --filecrc ' + " --e32input=" + root + ".dll")
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp)
                failed_tests+=1
                failed_sfx.append(sfx)
                if os.path.isfile(old):
                    os.remove(old)
            finally:
                print "\n"
                idx+=1

skipped_crc_sfx = []
skipped_dcrc_sfx = []
def AReaderSysdefDCRC(sfx):
    global skipped_dcrc_sfx
    if sfx in ('I', 'ID', 'IE', 'IN', 'IDi', 'UI', 'UID', 'UIE', 'UIN', 'UIDi', 'IDE', 'IDN', 'IDDi', 'IEN', 'IEDi', 'INDi', 'UIDE', 'UIDN', 'UIDDi', 'UIEN', 'UIEDi', 'UINDi', 'IDEN', 'IDEDi', 'IDNDi', 'IENDi', 'UIDEN', 'UIDEDi', 'UIDNDi', 'UIENDi', 'IDENDi', 'UIDENDi'):
        return "AlternateReaderRecog.SDK.dcrc"
    if sfx in ('U', 'D', 'E', 'N', 'Di', 'UD', 'UE', 'UN', 'UDi', 'DE', 'DN', 'DDi', 'EN', 'EDi', 'NDi', 'UDE', 'UDN', 'UDDi', 'UEN', 'UEDi', 'UNDi', 'DEN', 'DEDi', 'DNDi', 'ENDi', 'UDEN', 'UDEDi', 'UDNDi', 'UENDi', 'DENDi', 'UDENDi'):
        return os.path.join("testing_CRCs","ECOM.dcrc")
    skipped_dcrc_sfx.append("Unknown AReaderRecog DCRC: %s;" %sfx)
    return ""

def AReaderSysdefCRC(sfx):
    if sfx in ('I', 'ID', 'IE', 'UI', 'IDi', 'UID', 'UIE', 'UIDi', 'IDE', 'IDDi', 'IEDi', 'UIDE', 'UIDDi', 'UIEDi', 'IDEDi', 'UIDEDi'):
        return "AlternateReaderRecog.SDK.crc"
    if sfx in ('IN', 'UIN', 'IDN', 'IEN', 'INDi', 'UIDN', 'UIEN', 'UINDi', 'IDEN', 'IDNDi', 'IENDi', 'UIDEN', 'UIDNDi', 'UIENDi', 'IDENDi', 'UIDENDi'):
        return os.path.join("testing_CRCs","ECOM_IN.crc") 
    if sfx in ('D', 'E', 'U', 'Di', 'UD', 'UE', 'DE', 'DDi', 'EDi', 'UDE', 'UDi', 'UDDi', 'UEDi', 'DEDi', 'UDEDi'):
        return os.path.join("testing_CRCs","ECOM.crc")
    if sfx in ('N', 'DN', 'EN', 'UN', 'NDi', 'UDN', 'UEN', 'UNDi', 'DEN', 'DNDi', 'ENDi', 'UDEN', 'UDNDi', 'UENDi', 'DENDi', 'UDENDi'):
        return os.path.join("testing_CRCs","ECOM_N.crc")
    skipped_crc_sfx.append("Unknown AReaderRecog CRC: %s;" %sfx)
    return ""

def libcryptoDCRC(sfx):
    if sfx in ('U', 'IE', 'IN', 'EN', 'UIE', 'UIN', 'UEN', 'I', 'IEN', 'UIEN', 'E', 'N', 'UI', 'UE', 'UN'):
        return os.path.join("testing_CRCs","libcrypto_unfrozen.dcrc")
    if sfx in ('IDi', 'EDi', 'UIDi', 'UEDi', 'IEDi', 'UIEDi', 'Di', 'UDi'):
        return "libcrypto{000a0000}.dcrc"
# NDi - Namedlookup + Definput - this combo kills original el2e32
    if sfx in ("NDi", "UNDi", "INDi", "ENDi", "UINDi", "UENDi", "IENDi", "UIENDi"):
        return "libcrypto{000a0000}.dcrc"
    skipped_dcrc_sfx.append("Unknown libcrypto DCRC: %s;" %sfx)
    return ""

def libcryptoCRC(sfx):
    if sfx in ('U', 'I', 'E', 'UI', 'UE', 'IE', 'UIE'):
        return os.path.join("testing_CRCs","libcrypto_unfrozen.crc")
    if sfx in ('Di', 'UDi', 'IDi', 'EDi', 'UIDi', 'UEDi', 'IEDi', 'UIEDi'):
        return "libcrypto-2.4.5.SDK.crc"
    if sfx in ('N', 'UN', 'IN', 'EN', 'UIN', 'UEN', 'IEN', 'UIEN'):
        return os.path.join("testing_CRCs","libcrypto_namedlookup.crc")
# NDi - Namedlookup + Definput - this combo kills original el2e32
    if sfx in ("NDi", "UNDi", "INDi", "ENDi", "UINDi", "UENDi", "IENDi", "UIENDi"):
        return os.path.join("testing_CRCs","libcrypto_frozen_namedlookup.crc")
    skipped_crc_sfx.append("Unknown libcrypto CRC: %s;" %sfx)
    return ""

def PackedCRC(sfx):
    tmp = libcryptoDCRC(sfx)
    tmp1 = libcryptoCRC(sfx)
    if tmp != "" and tmp1 != "":
        return tmp+';'+tmp1
    if tmp == "":
        return tmp1
    return tmp

def PackedEcomCRC(sfx):
    tmp = AReaderSysdefDCRC(sfx)
    tmp1 = AReaderSysdefCRC(sfx)
    if tmp != "" and tmp1 != "":
        return tmp+';'+tmp1
    if tmp == "":
        return tmp1
    return tmp

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
    TortureECOM(elf2e32_SDK)
    ValidateECOM(elf2e32_SDK)
    E32WithFrozenDEF(elf2e32_SDK)
    # E32WithOutdatedDEF(elf2e32_SDK)
    PrintCRCDups()

def BuildAndValidateE32WithFrozenDEF():
    global failed_tests
    global failed_sfx
    crypto_dcrc = os.path.join("tmp","libcrypto{000a0000}.dcrc")
    crypto_crc = os.path.join("tmp","libcrypto{000a0000}.crc")

    cmdline = elf2e32_test + " " + libcrypto_opts + "--filecrc="
    idx = 1
    args = ['--unfrozen', '--ignorenoncallable', '--excludeunwantedexports', '--namedlookup', libcrypto_defin]
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            tmp, sfx, old, new = InitTest(addend, cmdline, idx, "libcrypto{000a0000}.dso", "frzn_(%02d)_TGT.dso", PackedCRC)
            # tmp+=" --force --uncompressed"
            try:
                subprocess.check_call(tmp)
                if os.path.isfile(new):
                    os.remove(new)
                os.rename(old, new)
                if os.path.isfile(crypto_dcrc):
                    new_dcrc = os.path.splitext(new)[0]
                    new_dcrc += ".dcrc"
                    if os.path.isfile(new_dcrc):
                        os.remove(new_dcrc)
                    os.rename(crypto_dcrc, new_dcrc)
                if os.path.isfile(crypto_crc):
                    new_crc = os.path.splitext(new)[0]
                    new_crc += ".crc"
                    if os.path.isfile(new_crc):
                        os.remove(new_crc)
                    os.rename(crypto_crc, new_crc)
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp)
                failed_tests+=1
                failed_sfx.append(sfx)
                try:
                    os.remove(old)
                except: pass
                pass
            finally:
                print "\n"
                idx+=1

def BuildAndValidateE32WithOutdatedDEF():
    global failed_tests
    global failed_sfx
    crypto_dcrc = os.path.join("tmp","libcrypto{000a0000}.dcrc")
    crypto_crc = os.path.join("tmp","libcrypto{000a0000}.crc")

    cmdline = elf2e32_test + " " + libcrypto_opts + "--filecrc="
    idx = 1
    args = ('--unfrozen', '--ignorenoncallable', '--excludeunwantedexports', '--namedlookup', ' --definput="libcryptou_openssl.def"')
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            tmp, sfx, old, new = InitTest(addend, cmdline, idx, "libcrypto{000a0000}.dso", "out_(%02d)_TGT.dso", PackedCRC)
            try:
                subprocess.check_call(tmp)
                if os.path.isfile(new):
                    os.remove(new)
                os.rename(old, new)
                if os.path.isfile(crypto_dcrc):
                    new_dcrc = os.path.splitext(new)[0]
                    new_dcrc += ".dcrc"
                    if os.path.isfile(new_dcrc):
                        os.remove(new_dcrc)
                    os.rename(crypto_dcrc, new_dcrc)
                if os.path.isfile(crypto_crc):
                    new_crc = os.path.splitext(new)[0]
                    new_crc += ".crc"
                    if os.path.isfile(new_crc):
                        os.remove(new_crc)
                    os.rename(crypto_crc, new_crc)
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp)
                failed_tests+=1
                failed_sfx.append(sfx)
                try:
                    os.remove(old)
                except: pass
                pass
            finally:
                print "\n"
                idx+=1


#valid error: elf2e32 : Error: E1036: Symbol XXX Missing from ELF File
def SkipMe2(sfx):
    # if sfx in ('Di', 'E', 'N', 'ID', 'IE', 'IN', 'DE', 'DN', 'EN', 'IDE', 'IDN', 'IEN', 'DEN', 'IDEN'):
    if sfx in ():
        return True
    return False

def BuildAndValidateECOM():
    global failed_tests
    global failed_sfx
    crypto_dcrc = os.path.join("tmp", "AlternateReaderRecog{000a0000}.dcrc")
    crypto_crc = os.path.join("tmp", "AlternateReaderRecog{000a0000}.crc")

    cmdline = elf2e32_test + " " + areader_opts + common_opts + " --sysdef=_Z24ImplementationGroupProxyRi,1; --filecrc="
    idx = 1
    args = ('--unfrozen', '--ignorenoncallable', '--dlldata', '--excludeunwantedexports', '--namedlookup', areader_defin)
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            tmp, sfx, old, new = InitTest(addend, cmdline, idx, "AlternateReaderRecog{000a0000}.dso", "ECOM_(%02d)_TGT.dso", PackedEcomCRC)
            # tmp += " --uncompressed --force"
            try:
                subprocess.check_call(tmp)
                if os.path.isfile(new):
                    os.remove(new)
                os.rename(old, new)
                if os.path.isfile(crypto_dcrc):
                    new_dcrc = os.path.splitext(new)[0]
                    new_dcrc += ".dcrc"
                    if os.path.isfile(new_dcrc):
                        os.remove(new_dcrc)
                    os.rename(crypto_dcrc, new_dcrc)
                if os.path.isfile(crypto_crc):
                    new_crc = os.path.splitext(new)[0]
                    new_crc += ".crc"
                    if os.path.isfile(new_crc):
                        os.remove(new_crc)
                    os.rename(crypto_crc, new_crc)
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp)
                failed_tests+=1
                failed_sfx.append(sfx)
                try:
                    os.remove(old)
                except: pass
                pass
            finally:
                print "\n"
                idx+=1

def TortureECOMDCRC(sfx):
    if sfx in ('U', 'UD', 'UE', 'UN', 'UDi', 'UDE', 'UDN', 'UDDi', 'UEN', 'UEDi', 'UNDi', 'UDEN', 'UDEDi', 'UDNDi', 'UENDi', 'UDENDi'):
        return os.path.join("testing_CRCs", "AR_torture_ECOM_I.dcrc")
    if sfx in ('UI', 'UID', 'UIE', 'UIN', 'UIDi', 'UIDE', 'UIDN', 'UIDDi', 'UIEN', 'UIEDi', 'UINDi', 'UIDEN', 'UIDEDi', 'UIDNDi', 'UIENDi', 'UIDENDi'):
        return os.path.join("testing_CRCs", "AR_torture_ECOM.dcrc")
    return ""

def TortureECOMCRC(sfx):
    if sfx in ('U', 'UD', 'UE', 'UDi', 'UDE', 'UDDi', 'UEDi', 'UDEDi'):
        return os.path.join("testing_CRCs", "AR_torture_ECOM_U.crc")
    if sfx in ('UI', 'UID', 'UIE', 'UIDi', 'UIDE', 'UIDDi', 'UIEDi', 'UIDEDi'):
        return os.path.join("testing_CRCs", "AR_torture_ECOM_UI.crc")
    if sfx in ('UN', 'UDN', 'UEN', 'UNDi', 'UDEN', 'UDNDi', 'UENDi', 'UDENDi'):
        return os.path.join("testing_CRCs", "AR_torture_ECOM_UN.crc")
    if sfx in ('UIN', 'UIDN', 'UIEN', 'UINDi', 'UIDEN', 'UIDNDi', 'UIENDi', 'UIDENDi'):
        return os.path.join("testing_CRCs", "AR_torture_ECOM_UIN.crc")
    return ""


    # if sfx in ('U', 'UI', 'UD', 'UE', 'UDi', 'UID', 'UIE', 'UIDi', 'UDE', 'UDDi', 'UEDi', 'UIDE', 'UIDDi', 'UIEDi', 'UDEDi', 'UIDEDi'):
        # return os.path.join("testing_CRCs", "AR_torture_ECOM.crc")
    # if sfx in ('UN', 'UIN', 'UDN', 'UEN', 'UNDi', 'UIDN', 'UIEN', 'UINDi', 'UDEN', 'UDNDi', 'UENDi', 'UIDEN', 'UIDNDi', 'UIENDi', 'UDENDi', 'UIDENDi'):
        # return os.path.join("testing_CRCs", "AR_torture_ECOM_N.crc")
    # return ""

def PackedTortureEcomCRC(sfx):
    dcrc = TortureECOMDCRC(sfx)
    crc = TortureECOMCRC(sfx)
    if dcrc != "" and crc != "":
        return dcrc+';'+crc
    if dcrc == "":
        return crc
    return dcrc

#valid error: elf2e32 : Error: E1036: Symbol XXX Missing from ELF File
def FrozenExports(addend):
    sfx = OptSuffix(addend)
    if sfx in ('Di', 'IDi', 'DDi', 'EDi', 'NDi', 'IDDi', 'IEDi', 'INDi', 'DEDi', 'DNDi', 'ENDi', 'IDEDi', 'IDNDi', 'IENDi', 'DENDi', 'IDENDi', 'I', 'D', 'E', 'N', 'ID', 'IE', 'IN', 'DE', 'DN', 'EN', 'IDE', 'IDN', 'IEN', 'DEN', 'IDEN'):
        return True
    return False


# --sysdef has argument to non-existed or "missed" function lala with ordinal 2
# --sysdef=_Z24ImplementationGroupProxyRi,1;lala,2;
# Expected result: all builds failed except unfrozen ones.
# Builded: 48/63.
# Expected: 32/63.
#
# SDK build:
# - DSO and DEF has 2 groups:
#   - U, UI, UD, UE, UN, UID, UIE, UIN, UDE, UDN, UEN, UIDE, UIDN, UIEN, UDEN, UIDEN - "missed" export added.
#   - Di, UDi, IDi, DDi, EDi, NDi, UIDi,  UDDi, UEDi, UNDi, IDDi, IEDi, INDi, DEDi, DNDi, ENDi, UIDDi, UIEDi, UINDi, UDEDi, UDNDi, UENDi, IDEDi, IDNDi, IENDi, DENDi, UIDEDi, UIDNDi, UIENDi, UDENDi, IDENDi, UIDENDi - "missed" export not added.
# - E32 has 2 groups:
#   - U, Di, UI, UD, UE, UDi, IDi, DDi, EDi, UID, UIE, UIDi, UDE, UDDi, UEDi, IDDi, IEDi, DEDi, UIDE, UDDi, UIEDi, UDEDi, IDEDi, UIDEDi, UIDNDi, UIENDi, UDENDi, IDENDi, UIDENDi
#   - UN, NDi, UIN, UDN, UEN, UNDi, INDi, DNDi, ENDi, UIDN, UIEN, UINDi, UDEN, UDNDi, UENDi, IDNDi, IENDi, DENDi, UIDEN
# First E32 group: "missed" function ignored, same as AlternateReaderRecog.SDK.dll
# Second E32 group:  "missed" function ignored, namedlookup section added.
# Resume: many *U* builds give combo DSO with 2 exports and E32 with 1 export. This is critical error.
# In this case, the test is regression. It is not a reference for the original version.
def BuildAndTortureECOM():
    global failed_tests
    global failed_sfx
    crypto_dcrc = os.path.join("tmp", "AlternateReaderRecog{000a0000}.dcrc")
    crypto_crc = os.path.join("tmp", "AlternateReaderRecog{000a0000}.crc")

    cmdline = elf2e32_test + " " + areader_opts + common_opts + " --sysdef=_Z24ImplementationGroupProxyRi,1;lala,2; --filecrc="
    idx = 1
    args = ('--unfrozen', '--ignorenoncallable', '--dlldata', '--excludeunwantedexports', '--namedlookup', areader_defin)
    for x in range(1, len(args)+1):
        for i in combinations(args, x):
            addend = ' '.join(i)
            if FrozenExports(addend):
               continue
            tmp, sfx, old, new = InitTest(addend, cmdline, idx, "AlternateReaderRecog{000a0000}.dso", "AR_(%02d)_TGT.dso", PackedTortureEcomCRC)
            # tmp += " --uncompressed --force"
            try:
                subprocess.check_call(tmp)
                if os.path.isfile(new):
                    os.remove(new)
                os.rename(old, new)
                if os.path.isfile(crypto_dcrc):
                    new_dcrc = os.path.splitext(new)[0]
                    new_dcrc += ".dcrc"
                    if os.path.isfile(new_dcrc):
                        os.remove(new_dcrc)
                    os.rename(crypto_dcrc, new_dcrc)
                if os.path.isfile(crypto_crc):
                    new_crc = os.path.splitext(new)[0]
                    new_crc += ".crc"
                    if os.path.isfile(new_crc):
                        os.remove(new_crc)
                    os.rename(crypto_crc, new_crc)
            except:
                print "Unexpectable test #%s failure:\n %s" %(idx, tmp)
                failed_tests+=1
                failed_sfx.append(sfx)
                try:
                    os.remove(old)
                except: pass
                pass
            finally:
                print "\n"
                idx+=1
    # PrintCRCDups()

def RenameFile(name):
    if(".SDK." in name):
        os.remove(name)
        return
    tmp = os.path.splitext(name)
    new = tmp[0] + ".SDK" + tmp[1]
    if os.path.isfile(new):
        os.remove(new)
    os.rename(name, new)

def RenameOutput():
    t = os.listdir("tmp")
    t = [os.path.join("tmp", x) for x in t]
    t = [x for x in t if os.path.isfile(x)]
    [RenameFile(x) for x in t]

def Run():
    # DeduceCRCS()
    # RenameOutput()
    BuildAndTortureECOM()
    BuildAndValidateECOM()
    BuildAndValidateE32WithFrozenDEF()
    # BuildAndValidateE32WithOutdatedDEF()

if __name__ == "__main__":
    # execute only if run as a script
    Run()
    if len(skipped_crc_sfx) > 0:
        print "skipped_crc_sfx:"
        print skipped_crc_sfx
    if len(skipped_dcrc_sfx) > 0:
        print "skipped_dcrc_sfx:"
        print skipped_dcrc_sfx
    if failed_tests > 0:
       print "Tests failed: %d/%d\n" %(failed_tests, started_tests)
       print failed_sfx
    else:
       print "Good Job! All DLL torture test passed! =D"
