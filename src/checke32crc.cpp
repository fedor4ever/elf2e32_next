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

 We store time of file creation because output E32Image should be identical. Else elf2e32 set actual time of file creation.

 If some section missed in .crc file their checksums has value 0xffffffff.

 There 2 way to load .crc files: explicit and implicit.

 Option "--filecrc=<filename>.crc" explicitly say what file to load.
    No output .crc file stored.

 Implicit use:
    1. E32Image used as input.
    Load .crc file near E32Image.
    Store .crc file near output E32Image if "--output" used.

    2. Elf file used as input when E32Image building.
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

struct E32Crcs
{
    static constexpr char FULLIMAGE[] = "fullimage";
    static constexpr char HEADER[] = "header";
    static constexpr char TIMELO[] = "timelo";
    static constexpr char TIMEHI[] = "timehi";
    static constexpr char HDR[] = "hdr";
    static constexpr char HDRJ[] = "hdrj";
    static constexpr char HDRV[] = "hdrv";
    static constexpr char EXPORTBITMAP[] = "exportbitmap";
    static constexpr char CODE[] = "code";
    static constexpr char DATA[] = "data";
    static constexpr char EXPORTS[] = "exports";
    static constexpr char SYMLOOK[] = "symlook";
    static constexpr char IMPORTS[] = "imports";
    static constexpr char CODERELOCS[] = "coderelocs";
    static constexpr char DATARELOCS[] = "datarelocs";
//    static constexpr char [] = "";
};

struct CRCData
{
    bool operator ==(const CRCData& right) const;
    bool operator !=(const CRCData& right) const;
    uint32_t iTimeLo;
    uint32_t iTimeHi;
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

// members ordered by place in E32Image
class E32CRC
{
    public:
        E32CRC(const E32Parser* parser, const Args* args);
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
    E32CRC crc(parser, args);
    crc.Run();
}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
void E32CRC::CRCToFile()
{
    stringstream buf;
    buf << E32Crcs::FULLIMAGE << " = " << iCRCOut.iFullImage << "\n";
    buf << E32Crcs::HEADER << " = " << iCRCOut.iHeader << "\n";
    buf << E32Crcs::TIMELO << " = " << iCRCOut.iTimeLo << "\n";
    buf << E32Crcs::TIMEHI << " = " << iCRCOut.iTimeHi << "\n";
    buf << E32Crcs::EXPORTBITMAP << " = " << iCRCOut.iExportBitMap << "\n";
    buf << E32Crcs::CODE << " = " << iCRCOut.iCode << "\n";
    buf << E32Crcs::DATA << " = " << iCRCOut.iData << "\n";
    buf << E32Crcs::EXPORTS << " = " << iCRCOut.iExports << "\n";
    buf << E32Crcs::SYMLOOK << " = " << iCRCOut.iSymlook << "\n";
    buf << E32Crcs::IMPORTS << " = " << iCRCOut.iImports << "\n";
    buf << E32Crcs::CODERELOCS << " = " << iCRCOut.iCodeRelocs << "\n";
    buf << E32Crcs::DATARELOCS << " = " << iCRCOut.iDataRelocs << "\n";
    SaveFile(iFileOut, buf.str());
}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
void E32CRC::ParseFile()
{
    fstream file(iFileIn, fstream::in);
    if(!file)
        return;

    string s;
    while(file.good())
    {
        getline(file, s);
        Tokenize(s);
    }

    if(!file)
        ReportError(FILEREADERROR, iFileIn);
    file.close();
}


/** @brief (one liner)
  *
  * (documentation goes here)
  */
void E32CRC::Tokenize(const string& line)
{
    stringstream stream(line);
    string type;
    char delim;
    uint32_t crc;
    stream >> type >> delim >> crc;

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
    else
        ReportError(ErrorCodes::ZEROBUFFER, "Error while parsing .crc file. Invalid data: " + line);
}

void E32CRC::DeduceCRCFiles()
{
    iFileIn = iArgs->iFileCrc;
    if(iFileIn.empty()) // option --filecrc not used
        return;

    iFileOut = iArgs->iOutput;
    CRCFile(iFileOut);

    if(iFileIn == DefaultOptionalArg)
        iFileIn = iArgs->iElfinput;
    if(iFileIn.empty())
        iFileIn = iArgs->iE32input;
    if(iFileIn.empty())
        ReportError(ErrorCodes::ZEROBUFFER, "Internal error in DeduceCRCFiles(). Got uninitialized FileIn object!");
    CRCFile(iFileIn);
}

void CRCFile(string& s)
{
    s.erase(s.find_last_of("."));
    s += ".crc";
}

/** @brief (one liner)
  *
  * Found CRC on E32Image
  */
void E32CRC::CRCsOnE32()
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
}

void PrintIfNEQ(uint32_t in, uint32_t out, const string& msg)
{
    if(in == out)
        return;
    std::stringstream s;
    s << msg + ": " << out << " - " << out << "\n";
    ReportLog(s.str());
}

/** @brief Compare predefined CRCs from .crc file and generated on E32Image
  *
  * Compare predefined CRCs from .crc file and generated on E32Image and print difference
  */
void E32CRC::PrintInvalidCRCs()
{
    if(iFileIn.empty())
        return;
    if(iCRCIn == iCRCOut)
    {
        if(iArgs->iVerbose)
            ReportLog("All CRC match!\n");
        return;
    }
    ReportWarning(ErrorCodes::ZEROBUFFER, "Found CRC32 mismatch(es) between in - out:");
    PrintIfNEQ(iCRCIn.iFullImage, iCRCOut.iFullImage, "FullImage");
    PrintIfNEQ(iCRCIn.iHeader, iCRCOut.iHeader, "Header");
    PrintIfNEQ(iCRCIn.iExportBitMap, iCRCOut.iExportBitMap, "ExportBitMap");
    PrintIfNEQ(iCRCIn.iCode, iCRCOut.iCode, "Code");
    PrintIfNEQ(iCRCIn.iData, iCRCOut.iData, "Data");
    PrintIfNEQ(iCRCIn.iExports, iCRCOut.iExports, "Exports");
    PrintIfNEQ(iCRCIn.iSymlook, iCRCOut.iSymlook, "Symlook");
    PrintIfNEQ(iCRCIn.iImports, iCRCOut.iImports, "Imports");
    PrintIfNEQ(iCRCIn.iCodeRelocs, iCRCOut.iCodeRelocs, "CodeRelocs");
    PrintIfNEQ(iCRCIn.iDataRelocs, iCRCOut.iDataRelocs, "DataRelocs");
//    PrintIfNEQ(iCRCIn., iCRCOut., "");
}

void E32CRC::Run()
{
    DeduceCRCFiles();
    ParseFile();
    CRCsOnE32();
    CRCToFile();
    PrintInvalidCRCs();
}

 E32CRC::E32CRC(const E32Parser* parser, const Args* args):
     iParser(parser), iArgs(args)
{
    iCrc = new E32Editor(parser);
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
//    if(this-> != right.)
//        return false;
    return true;
}

bool CRCData::operator !=(const CRCData& right) const
{
    return !(*this == right);
}
