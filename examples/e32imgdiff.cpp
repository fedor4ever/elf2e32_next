#include <string>
#include <iostream>

#include <fstream>
#include <sstream>

#include <cstdio>

#include "e32common.h"

using namespace std;

const uint32_t E32HeaderSize = sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ) + sizeof(E32ImageHeaderV);

bool operator==(const SSecurityInfo& comparator, const SSecurityInfo& comparable)
{
    if(comparator.iSecureId != comparable.iSecureId)
        return false;
    if(comparator.iVendorId != comparable.iVendorId)
        return false;
    if(comparator.iCaps != comparable.iCaps)
        return false;
    return true;
}

bool operator!=(const SSecurityInfo& comparator, const SSecurityInfo& comparable)
{
    return !(comparator == comparable);
}

bool operator==(const ToolVersion& comparator, const ToolVersion& comparable)
{
    if(comparator.iMajor != comparable.iMajor)
        return false;
    if(comparator.iMinor != comparable.iMinor)
        return false;
    if(comparator.iBuild != comparable.iBuild)
        return false;
    return true;
}

bool operator!=(const ToolVersion& comparator, const ToolVersion& comparable)
{
    return !(comparator == comparable);
}

bool operator==(const E32ImageHeader& comparator, const E32ImageHeader& comparable)
{
    if(comparator.iUid1 != comparable.iUid1)
        return false;
    if(comparator.iUid2 != comparable.iUid2)
        return false;
    if(comparator.iUid3 != comparable.iUid3)
        return false;
    if(comparator.iUidChecksum != comparable.iUidChecksum)
        return false;
    if(comparator.iHeaderCrc != comparable.iHeaderCrc)
        return false;
    if(comparator.iModuleVersion != comparable.iModuleVersion)
        return false;
    if(comparator.iCompressionType != comparable.iCompressionType)
        return false;
    if(comparator.iVersion != comparable.iVersion)
        return false;
    if(comparator.iTimeLo != comparable.iTimeLo)
        return false;
    if(comparator.iTimeHi != comparable.iTimeHi)
        return false;
    if(comparator.iFlags != comparable.iFlags)
        return false;
    if(comparator.iCodeSize != comparable.iCodeSize)
        return false;
    if(comparator.iDataSize != comparable.iDataSize)
        return false;
    if(comparator.iHeapSizeMin != comparable.iHeapSizeMin)
        return false;
    if(comparator.iHeapSizeMax != comparable.iHeapSizeMax)
        return false;
    if(comparator.iStackSize != comparable.iStackSize)
        return false;
    if(comparator.iBssSize != comparable.iBssSize)
        return false;
    if(comparator.iEntryPoint != comparable.iEntryPoint)
        return false;
    if(comparator.iCodeBase != comparable.iCodeBase)
        return false;
    if(comparator.iDataBase != comparable.iDataBase)
        return false;
    if(comparator.iDllRefTableCount != comparable.iDllRefTableCount)
        return false;
    if(comparator.iExportDirOffset != comparable.iExportDirOffset)
        return false;
    if(comparator.iExportDirCount != comparable.iExportDirCount)
        return false;
    if(comparator.iTextSize != comparable.iTextSize)
        return false;
    if(comparator.iCodeOffset != comparable.iCodeOffset)
        return false;
    if(comparator.iDataOffset != comparable.iDataOffset)
        return false;
    if(comparator.iImportOffset != comparable.iImportOffset)
        return false;
    if(comparator.iCodeRelocOffset != comparable.iCodeRelocOffset)
        return false;
    if(comparator.iDataRelocOffset != comparable.iDataRelocOffset)
        return false;
    if(comparator.iProcessPriority != comparable.iProcessPriority)
        return false;
    if(comparator.iCpuIdentifier != comparable.iCpuIdentifier)
        return false;
    return true;
}

bool operator==(const E32ImageHeaderJ& comparator, const E32ImageHeaderJ& comparable)
{
    if(comparator.iUncompressedSize != comparable.iUncompressedSize)
        return false;
    return true;
}

bool operator==(const E32ImageHeaderV& comparator, const E32ImageHeaderV& comparable)
{
    if(comparator.iS != comparable.iS)
        return false;
    if(comparator.iExceptionDescriptor != comparable.iExceptionDescriptor)
        return false;
    if(comparator.iSpare2 != comparable.iSpare2)
        return false;
    if(comparator.iExportDescSize != comparable.iExportDescSize)
        return false;
    if(comparator.iExportDescType != comparable.iExportDescType)
        return false;
    if(comparator.iExportDesc != comparable.iExportDesc)
        return false;
    return true;
}

void PrintIfNEQ(uint64_t in, uint64_t out, const string& msg)
{
    if(in == out)
        return;
    cout << std::hex << msg + ": 0x" << in << " - 0x" << out << "\n";
}

void getArg(string& arg)
{
    auto tmp = arg;
    auto argpos = arg.find_first_of("=");
    arg = tmp.substr(argpos + 1);
}

int main(int argc, char** argv)
{
    cout << "Hello user!" << endl;
    string comparator, comparable;
    bool iSDragNDrop = false;
    E32ImageHeader *e32comparator, *e32comparable;
    E32ImageHeaderJ *e32comparatorJ, *e32comparableJ;
    E32ImageHeaderV *e32comparatorV, *e32comparableV;

    string tmp;
    if(argc < 3)
    {
        cout << "Two arg required: 'comparator=file1' and  'comparable=file2'\n";
        cout << "Also you can drag-n-drop 2 files to app\n";
        std::cout << "Paused, press ENTER to continue." << std::endl;
        cin.get();
        return 0;
    }

    cout << "Starting E32 image header compare...\n" << endl;

    for(int i = 1; i < argc; i++) // skip filename
    {
        tmp = argv[i];
        if(tmp.find("comparator") != std::string::npos)
            comparator = tmp;
        if(tmp.find("comparable") != std::string::npos)
            comparable = tmp;
    }

    if(comparator.empty() && comparable.empty())
    {
        iSDragNDrop = true;
        for(int i = 1; i < argc; i++) // skip filename
        {
            tmp = argv[i];
            if(i == 1)
                comparator = tmp;
            if(i == 2)
                comparable = tmp;
        }
    }

    getArg(comparator);
    getArg(comparable);

//    read both files
    static char buf1[E32HeaderSize];
    static char buf2[E32HeaderSize];
    fstream file1(comparator, fstream::in);
    fstream file2(comparable, fstream::in);
    file1.read(buf1, E32HeaderSize);
    file2.read(buf2, E32HeaderSize);
    file1.close();
    file2.close();
//    use compare operator to print difference
    e32comparator = (E32ImageHeader*)buf1;
    e32comparable = (E32ImageHeader*)buf2;
    e32comparatorJ = (E32ImageHeaderJ*)(buf1 + sizeof(E32ImageHeader));
    e32comparableJ = (E32ImageHeaderJ*)(buf2 + sizeof(E32ImageHeader));
    e32comparatorV = (E32ImageHeaderV*)(buf1 + sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ));
    e32comparableV = (E32ImageHeaderV*)(buf2 + sizeof(E32ImageHeader) + sizeof(E32ImageHeaderJ));

    if( (e32comparator == e32comparable) && (e32comparatorJ == e32comparableJ) && (e32comparatorV == e32comparableV) )
    {
        cout << "File headers matches!\n";
        std::cout << "Paused, press ENTER to continue." << std::endl;
        cin.get();
        return 0;
    }

    cout << "Found mismatch(es) between in - out:\n";
    cout << comparator << " - " << comparable << "\n";
// E32ImageHeader
    PrintIfNEQ(e32comparator->iUid1,    e32comparable->iUid1, "E32ImageHeader::iUid1");
    PrintIfNEQ(e32comparator->iUid2,    e32comparable->iUid2, "E32ImageHeader::iUid2");
    PrintIfNEQ(e32comparator->iUid3,    e32comparable->iUid3, "E32ImageHeader::iUid3");
    PrintIfNEQ(e32comparator->iUidChecksum,   e32comparable->iUidChecksum, "E32ImageHeader::iUidChecksum");
    PrintIfNEQ(e32comparator->iHeaderCrc,     e32comparable->iHeaderCrc, "E32ImageHeader::iHeaderCrc");
    PrintIfNEQ(e32comparator->iModuleVersion,      e32comparable->iModuleVersion, "E32ImageHeader::iModuleVersion");
    PrintIfNEQ(e32comparator->iCompressionType,    e32comparable->iCompressionType, "E32ImageHeader::iCompressionType");
    PrintIfNEQ(e32comparator->iVersion.iMajor,    e32comparable->iVersion.iMajor, "E32ImageHeader::iVersion.iMajor");
    PrintIfNEQ(e32comparator->iVersion.iMinor,    e32comparable->iVersion.iMinor, "E32ImageHeader::iVersion.iMinor");
    PrintIfNEQ(e32comparator->iVersion.iBuild,    e32comparable->iVersion.iBuild, "E32ImageHeader::iVersion.iBuild");
    PrintIfNEQ(e32comparator->iTimeLo,   e32comparable->iTimeLo, "E32ImageHeader::iTimeLo");
    PrintIfNEQ(e32comparator->iTimeHi,   e32comparable->iTimeHi, "E32ImageHeader::iTimeHi");
    PrintIfNEQ(e32comparator->iFlags,    e32comparable->iFlags, "E32ImageHeader::iFlags");
    PrintIfNEQ(e32comparator->iCodeSize, e32comparable->iCodeSize, "E32ImageHeader::iCodeSize");
    PrintIfNEQ(e32comparator->iDataSize, e32comparable->iDataSize, "E32ImageHeader::iDataSize");
    PrintIfNEQ(e32comparator->iHeapSizeMin, e32comparable->iHeapSizeMin, "E32ImageHeader::iHeapSizeMin");
    PrintIfNEQ(e32comparator->iHeapSizeMax, e32comparable->iHeapSizeMax, "E32ImageHeader::iHeapSizeMax");
    PrintIfNEQ(e32comparator->iStackSize,   e32comparable->iStackSize, "E32ImageHeader::iStackSize");
    PrintIfNEQ(e32comparator->iBssSize,     e32comparable->iBssSize, "E32ImageHeader::iBssSize");
    PrintIfNEQ(e32comparator->iEntryPoint,  e32comparable->iEntryPoint, "E32ImageHeader::iEntryPoint");
    PrintIfNEQ(e32comparator->iCodeBase,    e32comparable->iCodeBase, "E32ImageHeader::iCodeBase");
    PrintIfNEQ(e32comparator->iDataBase,    e32comparable->iDataBase, "E32ImageHeader::iDataBase");
    PrintIfNEQ(e32comparator->iDllRefTableCount, e32comparable->iDllRefTableCount, "E32ImageHeader::iDllRefTableCount");
    PrintIfNEQ(e32comparator->iExportDirOffset,  e32comparable->iExportDirOffset, "E32ImageHeader::iExportDirOffset");
    PrintIfNEQ(e32comparator->iExportDirCount,   e32comparable->iExportDirCount, "E32ImageHeader::iExportDirCount");
    PrintIfNEQ(e32comparator->iTextSize,         e32comparable->iTextSize, "E32ImageHeader::iTextSize");
    PrintIfNEQ(e32comparator->iCodeOffset,    e32comparable->iCodeOffset, "E32ImageHeader::iCodeOffset");
    PrintIfNEQ(e32comparator->iDataOffset,    e32comparable->iDataOffset, "E32ImageHeader::iDataOffset");
    PrintIfNEQ(e32comparator->iImportOffset,  e32comparable->iImportOffset, "E32ImageHeader::iImportOffset");
    PrintIfNEQ(e32comparator->iCodeRelocOffset, e32comparable->iCodeRelocOffset, "E32ImageHeader::iCodeRelocOffset");
    PrintIfNEQ(e32comparator->iDataRelocOffset, e32comparable->iDataRelocOffset, "E32ImageHeader::iDataRelocOffset");
    PrintIfNEQ(e32comparator->iProcessPriority, e32comparable->iProcessPriority, "E32ImageHeader::iProcessPriority");
    PrintIfNEQ(e32comparator->iCpuIdentifier,   e32comparable->iCpuIdentifier, "E32ImageHeader::iCpuIdentifier");
// E32ImageHeaderJ
    PrintIfNEQ(e32comparatorJ->iUncompressedSize, e32comparableJ->iUncompressedSize, "E32ImageHeaderJ::iUncompressedSize");
// E32ImageHeaderV
    PrintIfNEQ(e32comparatorV->iS.iSecureId,    e32comparableV->iS.iSecureId, "E32ImageHeaderV::iS.iSecureId");
    PrintIfNEQ(e32comparatorV->iS.iVendorId,    e32comparableV->iS.iVendorId, "E32ImageHeaderV::iS.iVendorId");
    PrintIfNEQ(e32comparatorV->iS.iCaps,        e32comparableV->iS.iCaps, "E32ImageHeaderV::iS.iCaps");
    PrintIfNEQ(e32comparatorV->iExceptionDescriptor,    e32comparableV->iExceptionDescriptor, "E32ImageHeaderV::iExceptionDescriptor");
    PrintIfNEQ(e32comparatorV->iSpare2,            e32comparableV->iSpare2, "E32ImageHeaderV::iSpare2");
    PrintIfNEQ(e32comparatorV->iExportDescSize,    e32comparableV->iExportDescSize, "E32ImageHeaderV::iExportDescSize");
    PrintIfNEQ(e32comparatorV->iExportDescType,    e32comparableV->iExportDescType, "E32ImageHeaderV::iExportDescType");
    PrintIfNEQ(e32comparatorV->iExportDesc[0],        e32comparableV->iExportDesc[0], "E32ImageHeaderV::iExportDesc");

    if(iSDragNDrop)
    {
        std::cout << "Paused, press ENTER to continue." << std::endl;
        cin.get();
    }

    return 0;
}
