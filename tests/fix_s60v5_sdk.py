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
import os


path2sdk = "G:/Symbian/S60_5th_Edition_SDK_v1.0"

e32cmn_h_fix = r"""
#if __cplusplus >= 201103L
	TUint32 iUid;
#else
	TInt32 iUid;
#endif
"""

#Format: "file to edit" = "parent folder", src, dst
oneline_patch_src = [
("aknvolumecontrol.h", ["TInt CAknVolumeControl::ScaledValue() const;", "TInt ScaledValue() const;"]),
("cntdb.h", ["CContactConverter& CContactDatabase::ConverterL", "CContactConverter& ConverterL"]),
("rhttpsession.h", ["IMPORT_C const CCertificate* RHTTPSession::ServerCert();", "IMPORT_C const CCertificate* ServerCert();"]),
("imageprocessor.h", ["IMPORT_C static CImageProcessorExtension* ImageProcessorUtility::NewImageProcessorExtensionL", "IMPORT_C static CImageProcessorExtension* NewImageProcessorExtensionL"]),
("aiwvariant.h", ["IMPORT_C TAiwVariant& TAiwVariant::operator=", "IMPORT_C TAiwVariant& operator="]),
("sensoapmessage.h", ["IMPORT_C CSenSoapMessage::CSenSoapMessage();", "IMPORT_C CSenSoapMessage();"]),
("audiostreamengine.h", ["TPtr8& CAudioStreamEngine::GetFrame", "TPtr8& GetFrame"]),
("audiostreamengine.h", ["TPtr8& CAudioStreamEngine::GetPlaybackFrames", "TPtr8& GetPlaybackFrames"]),
("commondialogsexcontainer.h", [" void CCommonDialogsExContainer::ShowTextL", " void ShowTextL"]),
("lmuirefappview.h", ["TInt CLmUiRefAppView::LaunchSingleCategoeyDialogL();", "TInt LaunchSingleCategoeyDialogL();"]),
("snow.h", ["		CSnowfall::CSnowfall();", "		CSnowfall();"]),
("sensorexampleappview.h", ["void CSensorExampleAppView::GetPropertyStringL", "void GetPropertyStringL"]),
("xedomexample.h", ["void CXEDomEx::GetDriveLetterL();", "void GetDriveLetterL();"]),
("senxmlservicedescription.h", ["IMPORT_C void CSenXmlServiceDescription::SetPromptUserInfoL", "IMPORT_C void SetPromptUserInfoL"]), #not fixed
("examplerecognizer.mmp", ["systeminclude   \EPOC32", "systeminclude   \epoc32"]),
("sensoapmessage2.h", ["IMPORT_C CSenSoapMessage2::CSenSoapMessage2()", "IMPORT_C CSenSoapMessage2()"]),
("sipexresolverplugin.h", ["const TInt32 KSIPExResolverPluginUID", "const TUint32 KSIPExResolverPluginUID"]),
("Billboard.cpp", ["RequestToLoad( KGrassTexture, &iGrassTexture, false );", "RequestToLoad( KGrassTexture, &iGrassTexture );"]),
("Billboard.cpp", ["RequestToLoad( KCloudTexture, &iCloudTexture, false );", "RequestToLoad( KCloudTexture, &iCloudTexture );"]),
("Firefly.cpp", ["RequestToLoad( KFireFlyGlowName, &iGlowTexture, false );", "RequestToLoad( KFireFlyGlowName, &iGlowTexture );"]),
("BumpMap.cpp", ["RequestToLoad( KBumpMapName,     &iBumpmapTexture, false );", "RequestToLoad( KBumpMapName,     &iBumpmapTexture );"]),
("BumpMap.cpp", ["RequestToLoad( KTextureMapName,  &iTexture, false );", "RequestToLoad( KTextureMapName,  &iTexture );"]),
("simulateactiveobject.c", ["inline int Trace", "static inline int Trace"]),
("E32NpDllEntryPoint.cpp", ["const TInt KNpBitMapUidValue", "const TUint32 KNpBitMapUidValue"]),
("e32cmn.h", ["TInt32 iUid", e32cmn_h_fix]),
# ("", ["", ""]),
]

gcc_compiler_options = [
["calconexample.mmp", "\nOPTION GCCE -fpermissive\n"],
["pdrexample.mmp", "\nOPTION GCCE -fpermissive\n"],
["aknexpoplist.mmp", "\nLIBRARY         eikdlg.lib\n"],
["contacts.mmp", "\nLIBRARY         avkon.lib eikcoctl.lib eikctl.lib \n"],
# ["", "OPTION GCCE -fpermissive"]
# ["", ""],
]

def apply_patch(file, val):
   print "File to process: %s" %file
   with open(file) as d:
      x = d.read()
   if val[1] in x: #Already patched
      print "Already patched\n"
      return

   x = x.replace(val[0], val[1])
   if val[1] in x: #Already patched
      print "Patch for %s not applied!\n" %file
   with open(file, 'w') as f:
      f.write(x)
   print "Done!\n"

def one_line_patch(dst, root):
    for file, patch in oneline_patch_src:
        if file.lower() == dst.lower():
            file = os.path.join(root, file)
            apply_patch(file, patch)

def apply_gcc_patch(file, val):
   print "File to process: %s" %file
   with open(file) as d:
      x = d.read()
   if val in x: #Already patched
      print "Already patched\n"
      return

   with open(file, 'a') as f:
      f.write(val)
   print "Done!\n"

def fix_gcc_builds(dst, root):
    for val in gcc_compiler_options:
        file = val[0]
        patch = val[1]
        if file.lower() == dst.lower():
            file = os.path.join(root, file)
            apply_gcc_patch(file, patch)

def fix_symbian_headers():
    for root, dirs, files in os.walk(path2sdk, topdown=False):
        for name in files:
            one_line_patch(name, root)
            fix_gcc_builds(name, root)

if __name__ == "__main__":
    fix_symbian_headers()
