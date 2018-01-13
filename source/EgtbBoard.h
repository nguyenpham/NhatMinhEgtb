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

#ifndef EgtbBoard_h
#define EgtbBoard_h

#include "Egtb.h"

namespace egtb {

    extern const char* startingFen;
    extern const char* pieceTypeName;
    extern const int exchangePieceValue[6];

    class Piece {
    public:
        PieceType type;
        Side side;

        // for some purses such as pointing back to piece list
        int idx;

    public:
        Piece() {}
        Piece(PieceType _type, Side _side, int _idx = -1) {
            set(_type, _side, _idx);
        }

        void set(PieceType _type, Side _side, int _idx) {
            type = _type;
            side = _side;
            idx = _idx;

            assert(isValid());
        }

        void setEmpty() {
            set(PieceType::empty, Side::none, -1);
        }

        bool isEmpty() const {
            return type == PieceType::empty;
        }

        bool isPiece(PieceType _type, Side _side) const {
            return type == _type && side == _side;
        }

        bool same(int _idx) const {
            return idx == _idx;
        }

        bool operator == (const Piece& otherPiece) const {
            return idx == otherPiece.idx;
        }

        bool isValid() const {
            return (side == Side::none && type == PieceType::empty) || (side != Side::none && type != PieceType::empty);
        }

        static std::string toString(const PieceType type, const Side side) {
            int k = static_cast<int>(type);
            char ch = pieceTypeName[k];
            if (side == Side::white) {
                ch += 'A' - 'a';
            }
            return std::string(1, ch);
        }

        std::string toString() const {
            return toString(type, side);
        }
    };


    class Move {
    public:
        int from, dest;
        PieceType promote;

        PieceType type;
        Side side;

        Move() {}
        Move(PieceType _type, Side _side, int _from, int _dest, PieceType _promote = PieceType::empty) {
            set(_type, _side, _from, _dest, _promote);
        }
        Move(Side _side, int _from, int _dest, PieceType _promote = PieceType::empty) {
            set(_from, _dest, _side, _promote);
        }

        void set(PieceType _type, Side _side, int _from, int _dest, PieceType _promote = PieceType::empty) {
            type = _type;
            side = _side;
            from = _from;
            dest = _dest;
            promote = _promote;
        }

        void set(int _from, int _dest, Side _side, PieceType _promote) {
            from = _from;
            dest = _dest;
            side = _side;
            promote = _promote;
        }

        bool isValid() const {
            return from != dest  && from >= 0 && from < 64 && dest >= 0 && dest < 64;
        }

        std::string toString() const {
            std::ostringstream stringStream;
            stringStream << posToCoordinateString(from) << posToCoordinateString(dest);
            if (promote != PieceType::empty) {
                stringStream << "(" << Piece(promote, Side::white).toString() << ")";
            }
            return stringStream.str();
        }
    };

#define MaxMoveNumber 250

    class MoveList {
    public:
        Move list[MaxMoveNumber];
        int end;

        MoveList() {
            reset();
        }

        void reset() {
            end = 0;
        }

        bool isEmpty() const {
            return end == 0;
        }

        bool isFull() const {
            return end >= MaxMoveNumber - 2;
        }

        void add(const Move& move) {
            list[end] = move;
            end++;
        }

        void add(int from, int dest, Side side, PieceType promotion = PieceType::empty) {
            list[end].set(from, dest, side, promotion);
            end++;
        }

        void add(PieceType type, Side side, int from, int dest, PieceType promotion = PieceType::empty) {
            list[end].set(type, side, from, dest, promotion);
            end++;
        }

        bool isValid() const {
            return end >= 0 && end < MaxMoveNumber;
        }

        std::string toString() const {
            std::ostringstream stringStream;

            for (int i = 0; i < end; i++) {
                stringStream << i + 1 << ") " << list[i].toString() << " ";
            }
            return stringStream.str();
        }
    };

    class Hist {
    public:
        Move move;
        Piece movep, cap;
        int enpassant;
        int status;
        int8_t castleRights[2];

        bool repeatable;

        void set(const Move& _move) {
            move = _move;
        }

        bool isValid() const {
            return move.isValid() && cap.isValid();
        }

    };


    class EgtbBoardCore {
    public:
        Piece pieceList[2][16];
        Side side;

        int enpassant;
        int _status;
        int8_t castleRights[2];

    public:

        virtual void copy(const EgtbBoardCore& fromBoard) {
            enpassant = fromBoard.enpassant;
            _status = fromBoard._status;
            castleRights[0] = fromBoard.castleRights[0];
            castleRights[1] = fromBoard.castleRights[1];
            memcpy(&pieceList, &fromBoard.pieceList, sizeof(pieceList));
        }

        virtual void setPiece(int pos, Piece piece) = 0;
        virtual Piece getPiece(int pos) const = 0;
        virtual bool isPiece(int pos, PieceType type, Side side) const = 0;

        virtual bool isEmpty(int pos) const = 0;
        virtual void setEmpty(int pos) = 0;

        void reset() {
            for (int i = 0; i < 64; i++) {
                setEmpty(i);
            }
        }

        virtual void gen(MoveList& moveList, Side attackerSide, bool captureOnly) const { }
        virtual void genLegalOnly(MoveList& moveList, Side attackerSide, bool captureOnly = false);
        virtual bool isIncheck(Side beingAttackedSide) const;
        virtual bool beAttacked(int pos, Side attackerSide) const = 0;

        virtual void make(const Move& move, Hist& hist);
        virtual void takeBack(const Hist& hist);

        virtual bool isPositionValid(int pos) const {
            return pos >= 0 && pos < 64;
        }

        static int flip(int pos, FlipMode flipMode);
        static FlipMode flip(FlipMode oMode, FlipMode flipMode);

        void flip(FlipMode flipMode);

        bool isLegalEpCastle(int* ep, Side side);
        void checkEnpassant();

    protected:
        virtual void gen_addMove(MoveList& moveList, int from, int dest, bool capOnly) const;
        virtual void gen_addPawnMove(MoveList& moveList, int from, int dest, bool capOnly) const;
        virtual int findKing(Side side) const;
        virtual void clearCastleRights(int rookPos, Side rookSide);

    public:
        EgtbBoardCore();

        std::string toString() const;
        bool isValid() const;

        void setFen(const std::string& fen);
        std::string getFen(int halfCount = 0, int fullMoveCount = 1) const;
        void show() const;

    public:
        static void pieceList_reset(Piece *pieceList);
        static bool pieceList_set(Piece *pieceList, int pos, PieceType type, Side side);
        static bool pieceList_setEmpty(Piece *pieceList, int pos);
        static bool pieceList_setEmpty(Piece *pieceList, int pos, int sd);
        static bool pieceList_setEmpty(Piece *pieceList, int pos, PieceType type, Side side);
        static int  pieceList_countStrong(const Piece *pieceList, Side side);
        static bool pieceList_isDraw(const Piece *pieceList);

        static Side strongSide(const Piece *pieceList);

        bool pieceList_isDraw() const {
            return pieceList_isDraw((const Piece *)pieceList);
        }

        bool pieceList_make(const Hist& hist);
        bool pieceList_takeback(const Hist& hist);
        void pieceList_createList(Piece *pieceList) const;
        bool pieceList_setupBoard(const Piece *pieceList = nullptr);
    };


    ///////////////////////////////////////////////////
    class EgtbBoard : public EgtbBoardCore {
    protected:
        Piece pieces[64];

    public:
        void setPiece(int pos, Piece piece) {
            assert(isPositionValid(pos));
            pieces[pos] = piece;
        }

        Piece getPiece(int pos) const {
            assert(isPositionValid(pos));
            return pieces[pos];
        }

        bool isEmpty(int pos) const {
            assert(isPositionValid(pos));
            return pieces[pos].type == PieceType::empty;
        }

        bool isPiece(int pos, PieceType type, Side side) const {
            assert(isPositionValid(pos));
            auto p = pieces[pos];
            return p.type==type && p.side==side;
        }

        void setEmpty(int pos) {
            assert(isPositionValid(pos));
            pieces[pos].setEmpty();
        }
        bool setup(const std::vector<Piece> pieceVec, Side side, Squares enpassant = Squares::NoSquare);

        void gen(MoveList& moveList, Side side, bool capOnly) const;

        virtual bool beAttacked(int pos, Side attackerSide) const;

        void make(const Move& move, Hist& hist);
        void takeBack(const Hist& hist);

        int findKing(Side side) const {
            return pieceList[static_cast<int>(side)][0].idx;
        }

    };


} // namespace egtb

#endif /* EgtbBoard_h */

