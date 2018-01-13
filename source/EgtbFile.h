/*
 This file is part of NhatMinh Egtb, distributed under MIT license.

 Copyright (c) 2018 Nguyen Hong Pham

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#ifndef EgtbFile_h
#define EgtbFile_h

#include <assert.h>
#include <fstream>
#include <mutex>

#include "Egtb.h"


namespace egtb {

    class EgtbFileHeader {
    public:
        //*********** HEADER DATA ***************
        u16         signature;
        u32         property;
        u32         order;

        u8          dtm_max;
        u8          notused0;
        u8          notused[10];

        char        name[20], copyright[64];
        i64         checksum;
        char        reserver[80];
        //*********** END OF HEADER DATA **********

        void reset() {
            memset(this, 0, EGTB_HEADER_SIZE);
            signature = EGTB_ID_MAIN_V0;
        };

        bool isValid() const {
            return getVersion() >= 0;
        }

        int getVersion() const {
            switch (signature) {
                case EGTB_ID_MAIN_V0:
                    return 0;
            }
            return -1;
        }

        bool saveFile(std::ofstream& outfile) const {
            outfile.write ((char*)&signature, EGTB_HEADER_SIZE);
            return true;
        }

        bool readFile(std::ifstream& file) {
            if (file.read((char*)&signature, EGTB_HEADER_SIZE)) {
                return true;
            }
            return false;
        }

        bool isSide(Side side) const {
            return property & (1 << static_cast<int>(side));
        }

        void addSide(Side side) {
            property |= 1 << static_cast<int>(side);
        }

        void setOnlySide(Side side) {
            property &= ~((1 << W) | (1 << B));
            property |= 1 << static_cast<int>(side);
        }
    };

    /*
     * EGTB
     */
    class EgtbKeyRec;
    class EgtbFile
    {
    public:
        EgtbFileHeader *header;

        i64         size;

        char*       pBuf[2];

        u32*        compressBlockTables[2];
        char*       pCompressBuf;

        EgtbLoadStatus  loadStatus;

    protected:
        std::string path[2];

        void            reset();
        EgtbLoadMode    loadMode;

    public:
        i64         startpos[2], endpos[2];

        int         idxArr[8];
        i64         idxMult[32];
        EgtbMemMode memMode;

        std::string egtbName;

        bool        enpassantable;

        std::mutex  mtx;
        std::mutex  sdmtx[2];

        EgtbFile();
        ~EgtbFile();

        static bool knownExtension(const std::string& path);

        void    removeBuffers();

        i64     getSize() const { return size; }

        int getCompresseBlockCount() const {
            return (int)((getSize() + EGTB_SIZE_COMPRESS_BLOCK - 1) / EGTB_SIZE_COMPRESS_BLOCK);
        }
        bool    isCompressed() const { return header->property & EGTB_PROP_COMPRESSED; }

        int        getProperty() const { return header->property; }
        void    addProperty(int addprt) { header->property |= addprt; }

        void    setPath(const std::string& path, int sd);
        std::string getPath(int sd) const { return path[sd]; }

        std::string getName() const { assert(header == nullptr || egtbName == header->name); return egtbName; }

        i64     setupIdxComputing(const std::string& name, int order, int version);
        static i64 parseAttr(const std::string& name, int* idxArr, i64* idxMult, int* pieceCount, u16 order, int version);

        static i64 computeSize(const std::string &name);
        //        static i64 computeMaterialSigns(const std::string &name, u32 order);
        //        static i64 computeMaterialSigns(const std::string &name, int* idxArr, i64* idxMult, u16 order);

    protected:
        static i64 parseAttr(const int* idxArr, i64* idxMult, int* pieceCount, const int* orderArray, bool enpassantable);

        bool    readBuf(i64 startpos, int sd);
        bool    isDataReady(i64 pos, int sd) const { return pos >= startpos[sd] && pos < endpos[sd] && pBuf[sd]; }

        bool    createBuf(i64 len, int sd);

        i64     getBufItemCnt() const {
            if (memMode == EgtbMemMode::tiny) {
                return EGTB_SIZE_COMPRESS_BLOCK;
            }
            return getSize();
        }

        i64     getBufSize() const {
            return getBufItemCnt();
        }

        int    pieceCount[2][7];

        bool    isValid() const { return header->isValid() && pieceCount[0][0]==1 && pieceCount[1][0]==1; }

        //    public:
        //        int    materialsignWB, materialsignBW;

    public:
        virtual EgtbKeyRec getKey(const EgtbBoardCore& board) const;

        int     getScore(i64 idx, Side side, bool useLock = true);
        int     getScore(const EgtbBoardCore& board, Side side, bool useLock = true);

        virtual void    checkToLoadHeaderAndTable();

        bool    setupBoard(EgtbBoardCore& board, i64 idx, FlipMode flip, Side strongsider) const;

    protected:
        int     getScoreNoLock(i64 idx, Side side);
        int     getScoreNoLock(const EgtbBoardCore& board, Side side);

    public:
        bool    preload(const std::string& _path, EgtbMemMode mode, EgtbLoadMode loadMode);
        bool    loadHeaderAndTable(const std::string& path);
        virtual void    merge(EgtbFile& otherEgtbFile);

        int     cellToScore(char cell);

    protected:
        char    getCell(const EgtbBoardCore& board, Side side);
        char    getCell(i64 idx, Side side);

        bool    loadAllData(std::ifstream& file, Side side);
        bool    readCompressedBlock(std::ifstream& file, i64 idx, int sd, char* pDest);

        // May remove
    public:
        //        static u64 pieceListToMaterialSign(const Piece* pieceList);
        static std::string pieceListToName(const Piece* pieceList);

    };

} // namespace egtb

#endif /* EgtbFile_hpp */

