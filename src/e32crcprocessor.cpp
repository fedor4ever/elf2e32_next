 /*
 Copyright (c) 2023-2024 Strizhniou Fiodar
 All rights reserved.
 This component and the accompanying materials are made available
 under the terms of "Eclipse Public License v1.0"
 which accompanies this distribution, and is available
 at the URL "http://www.eclipse.org/legal/epl-v10.html".

 Initial Contributors:
 Strizhniou Fiodar - initial contribution.

 Contributors:

 Description:
 It's very important to check stability E32Image generation by elf2e32 while it's code base evolving.

 It creates CRC32 checksums for uncompressed E32Image and store them in file with ".crc" extension.
 Also verify generated E32Image between different versions if precalculated file used.

 That file stores precalculated CRC32 for different sections such as:
    - header
    - exportbitmap
    - CODE
    - DATA
    - EXPORTS
    - SYMLOOK
    - IMPORTS
    - CODERELOCS
    - DATARELOCS
    - image itself.
	- time of file creation

 We store some data that changes between builds because output E32Image
 should be identical. Else elf2e32 set actual time of file creation,
 it's own version, etc.
 If some section missed in .crc file their checksums has value 0xffffffff.

 There 2 way to load .crc files: explicit and implicit.

 Option "--filecrc=<filename>.crc" explicitly say what file to load.
    No output .crc file stored.

 Implicit use:
    1. E32Image used as input.
    Load .crc file near E32Image.

    2. E32Image used as input.
    Create .crc file near output E32Image if missed.

    3. Elf file used as input when E32Image building.
    Loads .crc near elf file.
    Store .crc file near output E32Image.
 */


#include <fstream>
#include <sstream>

#include "common.hpp"
#include "e32editor.h"
#include "e32parser.h"
#include "crcprocessor.h"
#include "elf2e32_opt.hpp"

using std::string;
using std::fstream;
using std::stringstream;

void CRCFile(string& s);

namespace E32Crcs
{
    static constexpr char FULLIMAGE[] = "fullimage";
    static constexpr char HEADER[] = "header";
    static constexpr char TIMELO[] = "timelo";
    static constexpr char TIMEHI[] = "timehi";
    static constexpr char EXPORTBITMAP[] = "exportbitmap";
    static constexpr char CODE[] = "code";
    static constexpr char DATA[] = "data";
    static constexpr char EXPORTS[] = "exports";
    static constexpr char SYMLOOK[] = "symlook";
    static constexpr char IMPORTS[] = "imports";
    static constexpr char CODERELOCS[] = "coderelocs";
    static constexpr char DATARELOCS[] = "datarelocs";
    static constexpr char FLAGS[] = "flags";
    static constexpr char VERSION_MAJOR[] = "version_major";
    static constexpr char VERSION_MINOR[] = "version_minor";
    static constexpr char VERSION_BUILD[] = "version_build";
    static constexpr char HEADERCRC[] = "headercrc";
    static constexpr char COMPRESSIONTYPE[] = "compressiontype";
    static constexpr char CAPS[] = "caps";
}

// stringstream treat uint8_t as a character not a number
struct CRCData
{
    bool operator ==(const CRCData& right) const;
    bool operator !=(const CRCData& right) const;
//    data that changes between builds
//    ordered by place in E32Image
    uint32_t iHeaderCrc = -1;
    uint32_t iCompressionType = -1; // do not check! Used to compare compressed and uncompressed builds
    uint32_t iVersion_Major = -1;
    uint32_t iVersion_Minor = -1;
    uint32_t iVersion_Build = -1;
    uint32_t iTimeLo;
    uint32_t iTimeHi;
    uint32_t iFlags = -1;
    uint64_t iCaps = -1;
//    checksums
    uint32_t iFullImage = -1;
    uint32_t iHeader = -1;
    uint32_t iExportBitMap = -1;
    uint32_t iCode = -1;
    uint32_t iData = -1;
    uint32_t iExports = -1;
    uint32_t iSymlook = -1;
    uint32_t iImports = -1;
    uint32_t iCodeRelocs = -1;
    uint32_t iDataRelocs = -1;
};

class E32CRCProcessor: public CRCProcessor
{
    public:
        E32CRCProcessor(const E32Parser* parser, const Args* args);
        virtual ~E32CRCProcessor();
    private:
        virtual void CRCFromFile() final override;
        virtual void SetCRCFiles() final override;
        virtual std::string CRCAsStr() final override;
        virtual void CRCFile(std::string& s) final override;
        virtual bool PrintInvalidTargetCRC() final override;
        virtual void ProcessTokens(const std::string&, uint32_t crc) final override;
    private:
        void FixE32Hdr();
    private:
        const E32Parser* iParser = nullptr;
        E32Editor* iCrc;
        CRCData iCRCIn;
        CRCData iCRCOut;
};

void CheckE32CRC(const E32Parser* parser, const Args* args)
{
    if(parser == nullptr)
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error in CheckE32CRC(). Got uninitialized E32Parser object!");
    if(args == nullptr)
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error in CheckE32CRC(). Got uninitialized Args object!");

    if(args->iFileCrc.empty()) // option --filecrc not used
        return;

    E32CRCProcessor crc(parser, args);
    crc.Run();
}

std::string E32CRCProcessor::CRCAsStr()
{
    stringstream buf;
    buf << std::hex;
    buf << E32Crcs::FULLIMAGE << " = 0x" << iCRCOut.iFullImage << "\n";
    buf << E32Crcs::HEADER << " = 0x" << iCRCOut.iHeader << "\n";
    buf << E32Crcs::TIMELO << " = 0x" << iCRCOut.iTimeLo << "\n";
    buf << E32Crcs::TIMEHI << " = 0x" << iCRCOut.iTimeHi << "\n";
    buf << E32Crcs::EXPORTBITMAP << " = 0x" << iCRCOut.iExportBitMap << "\n";
    buf << E32Crcs::CODE << " = 0x" << iCRCOut.iCode << "\n";
    buf << E32Crcs::DATA << " = 0x" << iCRCOut.iData << "\n";
    buf << E32Crcs::EXPORTS << " = 0x" << iCRCOut.iExports << "\n";
    buf << E32Crcs::SYMLOOK << " = 0x" << iCRCOut.iSymlook << "\n";
    buf << E32Crcs::IMPORTS << " = 0x" << iCRCOut.iImports << "\n";
    buf << E32Crcs::CODERELOCS << " = 0x" << iCRCOut.iCodeRelocs << "\n";
    buf << E32Crcs::DATARELOCS << " = 0x" << iCRCOut.iDataRelocs << "\n";
    buf << E32Crcs::FLAGS << " = 0x" << iCRCOut.iFlags << "\n";
    buf << E32Crcs::VERSION_MAJOR << " = 0x" << iCRCOut.iVersion_Major << "\n";
    buf << E32Crcs::VERSION_MINOR << " = 0x" << iCRCOut.iVersion_Minor << "\n";
    buf << E32Crcs::VERSION_BUILD << " = 0x" << iCRCOut.iVersion_Build << "\n";
    buf << E32Crcs::HEADERCRC << " = 0x" << iCRCOut.iHeaderCrc << "\n";
    buf << E32Crcs::COMPRESSIONTYPE << " = 0x" << iCRCOut.iCompressionType << "\n";
    buf << E32Crcs::CAPS << " = 0x" << iCRCOut.iCaps;
    return buf.str();
}

void E32CRCProcessor::ProcessTokens(const std::string& type, uint32_t crc)
{
    if(type == E32Crcs::FULLIMAGE)
        iCRCIn.iFullImage = crc;
    else if(type == E32Crcs::HEADER)
        iCRCIn.iHeader = crc;
    else if(type == E32Crcs::TIMELO)
        iCRCIn.iTimeLo = crc;
    else if(type == E32Crcs::TIMEHI)
        iCRCIn.iTimeHi = crc;
    else if(type == E32Crcs::EXPORTBITMAP)
        iCRCIn.iExportBitMap = crc;
    else if(type == E32Crcs::CODE)
        iCRCIn.iCode = crc;
    else if(type == E32Crcs::DATA)
        iCRCIn.iData = crc;
    else if(type == E32Crcs::EXPORTS)
        iCRCIn.iExports = crc;
    else if(type == E32Crcs::SYMLOOK)
        iCRCIn.iSymlook = crc;
    else if(type == E32Crcs::IMPORTS)
        iCRCIn.iImports = crc;
    else if(type == E32Crcs::CODERELOCS)
        iCRCIn.iCodeRelocs = crc;
    else if(type == E32Crcs::DATARELOCS)
        iCRCIn.iDataRelocs = crc;
    else if(type == E32Crcs::FLAGS)
        iCRCIn.iFlags = crc;
    else if(type == E32Crcs::VERSION_MAJOR)
        iCRCIn.iVersion_Major = crc;
    else if(type == E32Crcs::VERSION_MINOR)
        iCRCIn.iVersion_Minor = crc;
    else if(type == E32Crcs::VERSION_BUILD)
        iCRCIn.iVersion_Build = crc;
    else if(type == E32Crcs::HEADERCRC)
        iCRCIn.iHeaderCrc = crc;
    else if(type == E32Crcs::COMPRESSIONTYPE)
        iCRCIn.iCompressionType = crc;
    else if(type == E32Crcs::CAPS)
        iCRCIn.iCaps = crc;
    else
        ReportError(ErrorCodes::ZEROBUFFER, "Error while parsing .crc file. Invalid data: " + type);
}

void E32CRCProcessor::SetCRCFiles()
{
    if(!iArgs->iE32input.empty())
    {
        ReadOrCreateCRCFile(iArgs->iE32input);
        return;
    }

    if(iArgs->iOutput.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error in DeduceCRCFiles(). Got uninitialized E32 output!");
    if(iArgs->iElfinput.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error in DeduceCRCFiles(). Got uninitialized Elf input!");

    iFileIn = iArgs->iElfinput;
    iFileOut = iArgs->iOutput;
    CRCFile(iFileIn);
    CRCFile(iFileOut);
}

void E32CRCProcessor::CRCFile(string& s)
{
    s.erase(s.find_last_of("."));
    s += ".crc";
}

/// Generate CRC on E32Image
void E32CRCProcessor::CRCFromFile()
{
    FixE32Hdr();
    iCRCOut.iTimeLo = iCrc->TimeLo();
    iCRCOut.iTimeHi = iCrc->TimeHi();
    iCRCOut.iFullImage = iCrc->FullImage();
    iCRCOut.iHeader = iCrc->Header();
    iCRCOut.iExportBitMap = iCrc->ExportBitMap();
    iCRCOut.iCode = iCrc->Code();
    iCRCOut.iData = iCrc->Data();
    iCRCOut.iExports = iCrc->Exports();
    iCRCOut.iSymlook = iCrc->Symlook();
    iCRCOut.iImports = iCrc->Imports();
    iCRCOut.iCodeRelocs = iCrc->CodeRelocs();
    iCRCOut.iDataRelocs = iCrc->DataRelocs();
    iCRCOut.iFlags = iCrc->Flags();
    iCRCOut.iHeaderCrc = iCrc->HeaderCrc();
    iCRCOut.iCompressionType = iCrc->CompressionType();
    iCRCOut.iVersion_Major = iCrc->Version_Major();
    iCRCOut.iVersion_Minor = iCrc->Version_Minor();
    iCRCOut.iVersion_Build = iCrc->Version_Build();
    iCRCOut.iCaps = iCrc->Caps();
}

/** @brief Print difference for CRCs if any.
  *
  * Print difference between predefined CRCs from .crc file and generated from E32Image if any.
  */
bool E32CRCProcessor::PrintInvalidTargetCRC()
{
    ReportLog("E32CRCProcessor: ");
    if(iCRCIn == iCRCOut)
    {
        ReportLog("All E32 CRC matches!\n");
        return false;
    }
    ReportWarning(ErrorCodes::ZEROBUFFER, "Found CRC32 mismatch(es) between in - out:\n");
    PrintIfNEQ(iCRCIn.iFullImage,    iCRCOut.iFullImage, "FullImage");
    PrintIfNEQ(iCRCIn.iHeader,       iCRCOut.iHeader, "Header");
    PrintIfNEQ(iCRCIn.iExportBitMap, iCRCOut.iExportBitMap, "ExportBitMap");
    PrintIfNEQ(iCRCIn.iCode,    iCRCOut.iCode, "Code");
    PrintIfNEQ(iCRCIn.iData,    iCRCOut.iData, "Data");
    PrintIfNEQ(iCRCIn.iExports, iCRCOut.iExports, "Exports");
    PrintIfNEQ(iCRCIn.iSymlook, iCRCOut.iSymlook, "Symlook");
    PrintIfNEQ(iCRCIn.iImports, iCRCOut.iImports, "Imports");
    PrintIfNEQ(iCRCIn.iCodeRelocs, iCRCOut.iCodeRelocs, "CodeRelocs");
    PrintIfNEQ(iCRCIn.iDataRelocs, iCRCOut.iDataRelocs, "DataRelocs");
    PrintIfNEQ(iCRCIn.iFlags, iCRCOut.iFlags, "Flags");
    PrintIfNEQ(iCRCIn.iVersion_Major, (uint32_t)iCRCOut.iVersion_Major, "Version_iMajor");
    PrintIfNEQ(iCRCIn.iVersion_Minor, iCRCOut.iVersion_Minor, "Version_iMinor");
    PrintIfNEQ(iCRCIn.iVersion_Build, iCRCOut.iVersion_Build, "Version_iBuild");
    PrintIfNEQ(iCRCIn.iHeaderCrc, iCRCOut.iHeaderCrc, "HeaderCrc");
//    PrintIfNEQ(iCRCIn.iCompressionType, iCRCOut.iCompressionType, "CompressionType"); // do not check!
    PrintIfNEQ(iCRCIn.iVersion_Build, iCRCOut.iVersion_Build, "HeaderCrc");
    return true;
}

E32CRCProcessor::E32CRCProcessor(const E32Parser* parser, const Args* args):
     CRCProcessor(args, ".crc"), iParser(parser)
{
    iCrc = E32Editor::NewL(parser);
}

E32CRCProcessor::~E32CRCProcessor()
{
    delete iCrc;
}

void E32CRCProcessor::FixE32Hdr()
{
    if(!IsFileExist(iFileIn))
        return;
// For example in manual E32Image builds comression maybe off for easy hex view.
// This change header and even entire file checksums and test failed.
#ifndef SET_COMPILETIME_LOAD_EXISTED_FILECRC
    iCrc->SetCaps(iCRCIn.iCaps);
    iCrc->SetFlags(iCRCIn.iFlags);
#endif // SET_COMPILETIME_LOAD_EXISTED_FILECRC
    iCrc->SetCompressionType(iCRCIn.iCompressionType);
    iCrc->SetE32Time(iCRCIn.iTimeLo, iCRCIn.iTimeHi);
    iCrc->SetVersion(iCRCIn.iVersion_Major, iCRCIn.iVersion_Minor, iCRCIn.iVersion_Build);
    iCrc->ReGenerateCRCs();
    iCrc->DumpE32Img();
}


bool CRCData::operator ==(const CRCData& right) const
{
    if (this == std::addressof(right))
        return true;

    if(this->iTimeLo != right.iTimeLo)
        return false;
    if(this->iTimeHi != right.iTimeHi)
        return false;
    if(this->iFullImage != right.iFullImage)
        return false;
    if(this->iHeader != right.iHeader)
        return false;
    if(this->iExportBitMap != right.iExportBitMap)
        return false;
    if(this->iCode != right.iCode)
        return false;
    if(this->iData != right.iData)
        return false;
    if(this->iExports != right.iExports)
        return false;
    if(this->iSymlook != right.iSymlook)
        return false;
    if(this->iImports != right.iImports)
        return false;
    if(this->iCodeRelocs != right.iCodeRelocs)
        return false;
    if(this->iDataRelocs != right.iDataRelocs)
        return false;
    if(this->iFlags != right.iFlags)
        return false;
    if(this->iVersion_Major != right.iVersion_Major)
        return false;
    if(this->iVersion_Minor != right.iVersion_Minor)
        return false;
    if(this->iVersion_Build != right.iVersion_Build)
        return false;
    if(this->iHeaderCrc != right.iHeaderCrc)
        return false;
//    if(this->iCompressionType != right.iCompressionType)
//        return false; // do not check!
    if(this->iCaps != right.iCaps)
        return false;
    return true;
}

bool CRCData::operator !=(const CRCData& right) const
{
    return !(*this == right);
}
