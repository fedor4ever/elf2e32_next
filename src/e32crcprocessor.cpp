 /*
 Copyright (c) 2023 Strizhniou Fiodar
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

class E32CRCProcessor
{
    public:
        E32CRCProcessor(const E32Parser* parser, const Args* args);
        ~E32CRCProcessor();
        void Run();
    private:
        void DeduceCRCFiles();
        void ParseFile();
        void CRCToFile();
        void Tokenize(const string& line);
        void CRCsOnE32();
        void PrintInvalidCRCs();
    private:
        const E32Parser* iParser = nullptr;
        const Args* iArgs = nullptr;
        E32Editor* iCrc;
        CRCData iCRCIn;
        CRCData iCRCOut;
        string iFileIn;
        string iFileOut;
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

void E32CRCProcessor::CRCToFile()
{
    if(iFileOut.empty())
        return;

    ReportLog("Saving checksums to file: " + iFileOut + "\n\n");
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
    buf << E32Crcs::CAPS << " = 0x" << iCRCOut.iCaps;

    SaveFile(iFileOut, buf.str());
}

void E32CRCProcessor::ParseFile()
{
    fstream file(iFileIn, fstream::in);
    if(!file)
        return;

    ReportLog("Reading checksums from file: " + iFileIn + "\n\n");

    string s;
    while(file.good())
    {
        getline(file, s);
        Tokenize(s);
    }

    if(!file)
        ReportError(FILEREADERROR, iFileIn);
    file.close();

    iCrc->SetCaps(iCRCIn.iCaps);
    iCrc->SetFlags(iCRCIn.iFlags);
    iCrc->SetHeaderCrc(iCRCIn.iHeaderCrc);
    iCrc->SetE32Time(iCRCIn.iTimeLo, iCRCIn.iTimeHi);
    iCrc->SetVersion(iCRCIn.iVersion_Major, iCRCIn.iVersion_Minor, iCRCIn.iVersion_Build);
}

void E32CRCProcessor::Tokenize(const string& line)
{
    if(line.empty())
        return;

    stringstream stream(line);
    string type;
    char delim;
    uint32_t crc;
    stream >> std::hex >> type >> delim >> crc;

    if(delim != '=')
        ReportError(ErrorCodes::ZEROBUFFER, "Error while parsing .crc file. Delimeter '=' not found!");

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
    else if(type == E32Crcs::CAPS)
        iCRCIn.iCaps = crc;
    else
        ReportError(ErrorCodes::ZEROBUFFER, "Error while parsing .crc file. Invalid data: " + line);
}

void E32CRCProcessor::DeduceCRCFiles()
{
    iFileIn = iArgs->iFileCrc;
    if(iFileIn != DefaultOptionalArg)
        return;

    if(!iArgs->iE32input.empty())
    {
        iFileIn = iArgs->iE32input;
        CRCFile(iFileIn);
        fstream file(iFileIn, fstream::in);
        if(!file)
        {
            iFileOut = iFileIn;
            iFileIn.clear();
        }
        file.close();
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

#ifdef SET_COMPILETIME_LOAD_EXISTED_FILECRC
    fstream file(iFileOut, fstream::in);
    if(file.is_open())
    {
        std::swap(iFileIn, iFileOut);
        iFileOut.clear();
    }
#endif // SET_COMPILETIME_LOAD_EXISTED_FILECRC
}

void CRCFile(string& s)
{
    s.erase(s.find_last_of("."));
    s += ".crc";
}

/// Generate CRC on E32Image
void E32CRCProcessor::CRCsOnE32()
{
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
    iCRCOut.iVersion_Major = iCrc->Version_Major();
    iCRCOut.iVersion_Minor = iCrc->Version_Minor();
    iCRCOut.iVersion_Build = iCrc->Version_Build();
    iCRCOut.iCaps = iCrc->Caps();
}

void PrintIfNEQ(uint32_t in, uint32_t out, const string& msg)
{
    if(in == out)
        return;
    std::stringstream s;
    s << std::hex << msg + ": 0x" << in << " - 0x" << out << "\n";
    ReportLog(s.str());
}

/** @brief Print difference for CRCs if any.
  *
  * Print difference between predefined CRCs from .crc file and generated from E32Image if any.
  */
void E32CRCProcessor::PrintInvalidCRCs()
{
    if(iFileIn.empty())
        return;

    ReportLog("E32CRCProcessor: ");
    if(iCRCIn == iCRCOut)
    {
        ReportLog("All CRC matches!\n");
        return;
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
    PrintIfNEQ(iCRCIn.iVersion_Build, iCRCOut.iVersion_Build, "HeaderCrc");
    ReportLog("\n");
}

E32CRCProcessor::E32CRCProcessor(const E32Parser* parser, const Args* args):
     iParser(parser), iArgs(args)
{
    iCrc = E32Editor::NewL(parser);
}

E32CRCProcessor::~E32CRCProcessor()
{
    delete iCrc;
}

void E32CRCProcessor::Run()
{
    DeduceCRCFiles();
    ParseFile();
    CRCsOnE32();
    CRCToFile();
    PrintInvalidCRCs();
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
    if(this->iCaps != right.iCaps)
        return false;
    return true;
}

bool CRCData::operator !=(const CRCData& right) const
{
    return !(*this == right);
}
