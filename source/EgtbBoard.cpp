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

#include "EgtbBoard.h"

namespace egtb {
    extern const char* pieceTypeName;
    extern const int exchangePieceValue[6];
    extern const char* startingFen;

    const char* pieceTypeName = "kqrbnp.";

    const int exchangePieceValue[6] = { 10000, 1100, 500, 300, 250, 100 };

    const char* startingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - - -";
}

using namespace egtb;


EgtbBoardCore::EgtbBoardCore() {
}

bool EgtbBoardCore::isValid() const {
    int pieceCout[2][6] = { {0,0,0,0,0,0}, {0,0,0,0,0,0} };

    for (int i = 0; i < 64; i++) {
        auto piece = getPiece(i);
        if (piece.isEmpty()) {
            continue;
        }

        pieceCout[static_cast<int>(piece.side)][static_cast<int>(piece.type)] += 1;
        if (piece.type == PieceType::pawn) {
            if (i < 8 || i >= 56) {
                return false;
            }
        }
    }

    if (castleRights[0] + castleRights[1]) {
        if (castleRights[B]) {
            if (!isPiece(4, PieceType::king, Side::black)) {
                return false;
            }
            if (((castleRights[B] & CASTLERIGHT_LONG) && !isPiece(0, PieceType::rook, Side::black)) ||
                ((castleRights[B] & CASTLERIGHT_SHORT) && !isPiece(7, PieceType::rook, Side::black))) {
                return false;
            }
        }

        if (castleRights[W]) {
            if (!isPiece(60, PieceType::king, Side::white)) {
                return false;
            }
            if (((castleRights[W] & CASTLERIGHT_LONG) && !isPiece(56, PieceType::rook, Side::white)) ||
                ((castleRights[W] & CASTLERIGHT_SHORT) && !isPiece(63, PieceType::rook, Side::white))) {
                return false;
            }
        }
    }

    if (enpassant > 0) {
        auto row = ROW(enpassant);
        if (row != 2 && row != 5) {
            return false;
        }
        auto pawnPos = row == 2 ? (enpassant + 8) : (enpassant - 8);
        if (!isPiece(pawnPos, PieceType::pawn, row == 2 ? Side::black : Side::white)) {
            return false;
        }
    }

    bool b =
    pieceCout[0][0] == 1 && pieceCout[1][0] == 1 &&     // king
    pieceCout[0][1] <= 9 && pieceCout[1][1] <= 9 &&     // queen
    pieceCout[0][2] <= 10 && pieceCout[1][2] <= 10 &&     // rook
    pieceCout[0][3] <= 10 && pieceCout[1][3] <= 10 &&     // bishop
    pieceCout[0][4] <= 10 && pieceCout[1][4] <= 10 &&     // knight
    pieceCout[0][5] <= 8 && pieceCout[1][5] <= 8 &&       // pawn
    pieceCout[0][1]+pieceCout[0][2]+pieceCout[0][3]+pieceCout[0][4]+pieceCout[0][5] <= 15 &&
    pieceCout[1][1]+pieceCout[1][2]+pieceCout[1][3]+pieceCout[1][4]+pieceCout[1][5] <= 15;

    assert(b);
    return b;
}

void EgtbBoardCore::checkEnpassant() {
    if ((enpassant >= 16 && enpassant < 24) || (enpassant >= 40 && enpassant < 48))  {
        int d = 8, xsd = W, r = 3;
        if (enpassant >= 40) {
            d = -8; xsd = B; r = 4;
        }

        for(int i = 8; i < 16; i++) {
            auto p = pieceList[xsd][i];
            if (p.type == PieceType::pawn && (p.idx == enpassant + d - 1 || p.idx == enpassant + d + 1) && ROW(p.idx) == r) {
                return;
            }
        }
    }
    enpassant = -1;
}

bool EgtbBoardCore::isLegalEpCastle(int* ep, Side side) {
    bool r = true;
    int sd = static_cast<int>(side);
    enpassant = ep[sd];

    if (enpassant >= 16) {
        int d = 8;
        auto side = Side::white;
        if (enpassant >= 40) {
            d = -8; side = Side::black;
        }

        int epCnt = 0, attackerCnt = 0;

        for (int sd = 0; sd < 2 && epCnt >= 0; sd++) {
            for(int i = 0; i < 16; i++) {
                auto p = pieceList[sd][i];
                if (p.isEmpty()) {
                    continue;
                }
                // Not empty
                if (p.idx == enpassant || p.idx == enpassant - d) {
                    epCnt = -1;
                    break;
                }

                if (p.idx == enpassant + d) {
                    if (p.type != PieceType::pawn || p.side != side) {
                        epCnt = -1;
                        break;
                    }
                    epCnt++;
                    continue;
                }
                if ((p.idx == enpassant + d - 1 || p.idx == enpassant + d + 1) && p.type == PieceType::pawn && p.side != side) {
                    attackerCnt++;
                }
            }
        }

        if (epCnt != 1 && attackerCnt < 1) {
            enpassant = 0;
            r = false;
        }
    }

    for (int s = 0; s < 2; s++) {
        if (!castleRights[s]) {
            continue;
        }
        int n = 0;
        if (castleRights[s] & CASTLERIGHT_SHORT) {
            n++;
        }
        if (castleRights[s] & CASTLERIGHT_LONG) {
            n++;
        }

        int correct = 0;
        int kingPos = pieceList[s][0].idx;
        if ((s == W && kingPos == 60) || (s == B && kingPos == 4)) {
            for(int i = 1; i < 16; i++) {
                const Piece& p = pieceList[s][i];
                if (p.type != PieceType::rook) {
                    continue;
                }

                if (p.idx == kingPos + 3) { // Short
                    if (castleRights[s] & CASTLERIGHT_SHORT) {
                        correct++;
                        if (correct == n) {
                            continue;
                        }
                    }
                }
                if (p.idx == kingPos - 4) { // Long
                    if (castleRights[s] & CASTLERIGHT_LONG) {
                        correct++;
                        if (correct == n) {
                            continue;
                        }
                    }
                }
            }
        }
        if (n != correct) {
            r = false;
        }
    }

    return r;
}

void EgtbBoardCore::show() const {
    std::cout << toString() << std::endl;
}

std::string EgtbBoardCore::toString() const {

    std::ostringstream stringStream;

    stringStream << getFen() << std::endl;

    for (int i = 0; i<64; i++) {
        auto piece = getPiece(i);

        stringStream << Piece::toString(piece.type, piece.side) << " ";

        if (i > 0 && COL(i) == 7) {
            int row = 8 - ROW(i);
            stringStream << " " << row << "\n";
        }
    }

    stringStream << "a b c d e f g h \n"; // << (side == Side::white ? "w turns" : "b turns") << "\n";

    return stringStream.str();
}

void EgtbBoardCore::setFen(const std::string& fen) {
    pieceList_reset((Piece *)pieceList);
    reset();

    std::string thefen = fen;
    if (fen.empty()) {
        thefen = startingFen;
    }

    bool last = false;
    side = Side::none;
    enpassant = -1;
    _status = 0;
    castleRights[0] = castleRights[1] = 0;

    for (int i = 0, pos = 0; i < thefen.length(); i++) {
        char ch = thefen.at(i);

        if (ch==' ') {
            last = true;
            continue;
        }

        if (last) {
            // enpassant
            if (ch >= 'a' && ch <= 'h' && i + 1 < thefen.length()) {
                char ch2 = thefen.at(i + 1);
                if (ch2 >= '1' && ch2 <= '8') {
                    enpassant = (7 - (ch2 - '1')) * 8 + (ch - 'a');
                    continue;
                }
            }

            switch (ch) {
                case 'w':
                case 'W':
                    side = Side::white;
                    break;
                case 'b':
                case 'B':
                    side = Side::black;
                    break;
                case 'K':
                    castleRights[W] |= CASTLERIGHT_SHORT;
                    break;
                case 'k':
                    castleRights[B] |= CASTLERIGHT_SHORT;
                    break;
                case 'Q':
                    castleRights[W] |= CASTLERIGHT_LONG;
                    break;
                case 'q':
                    castleRights[B] |= CASTLERIGHT_LONG;
                    break;

                default:
                    break;
            }

            continue;
        }

        if (ch=='/') {
            std::string str = fen.substr();
            continue;
        }

        if (ch>='0' && ch <= '8') {
            int num = ch - '0';
            pos += num;
            continue;
        }

        Side side = Side::black;
        if (ch >= 'A' && ch < 'Z') {
            side = Side::white;
            ch += 'a' - 'A';
        }

        auto pieceType = PieceType::empty;
        const char* p = strchr(pieceTypeName, ch);
        if (p != NULL) {
            int k = (int)(p - pieceTypeName);
            pieceType = static_cast<PieceType>(k);

        }
        if (pieceType != PieceType::empty) {
            setPiece(pos, Piece(pieceType, side));
            pieceList_set((Piece *)pieceList, pos, pieceType, side);
        }
        pos++;
    }

    checkEnpassant();
}

std::string EgtbBoardCore::getFen(int halfCount, int fullMoveCount) const {
    std::ostringstream stringStream;

    int e = 0;
    for (int i=0; i < 64; i++) {
        auto piece = getPiece(i);
        if (piece.isEmpty()) {
            e += 1;
        } else {
            if (e) {
                stringStream << e;
                e = 0;
            }
            stringStream << piece.toString();
        }

        if (i % 8 == 7) {
            if (e) {
                stringStream << e;
            }
            if (i < 63) {
                stringStream << "/";
            }
            e = 0;
        }
    }

    stringStream << (side == Side::white ? " w " : " b ");

    if (castleRights[W] + castleRights[B]) {
        if (castleRights[W] & CASTLERIGHT_SHORT) {
            stringStream << "K";
        }
        if (castleRights[W] & CASTLERIGHT_LONG) {
            stringStream << "Q";
        }
        if (castleRights[B] & CASTLERIGHT_SHORT) {
            stringStream << "k";
        }
        if (castleRights[B] & CASTLERIGHT_LONG) {
            stringStream << "q";
        }
    } else {
        stringStream << "--";
    }

    stringStream << " " << halfCount << " " << fullMoveCount;

    return stringStream.str();
}

void EgtbBoardCore::gen_addMove(MoveList& moveList, int from, int dest, bool captureOnly) const
{
    auto toSide = getPiece(dest).side;
    Piece movingPiece = getPiece(from);
    auto fromSide = movingPiece.side;

    if (fromSide != toSide && (!captureOnly || toSide != Side::none)) {
        moveList.add(Move(movingPiece.type, fromSide, from, dest));
    }
}

void EgtbBoardCore::gen_addPawnMove(MoveList& moveList, int from, int dest, bool captureOnly) const
{
    auto toSide = getPiece(dest).side;
    auto fromSide = getPiece(from).side;

    if (fromSide != toSide && (!captureOnly || toSide != Side::none)) {
        if (dest >= 8 && dest < 56) {
            moveList.add(Move(PieceType::pawn, fromSide, from, dest));
        } else {
            moveList.add(Move(PieceType::pawn, fromSide, from, dest, PieceType::queen));
            moveList.add(Move(PieceType::pawn, fromSide, from, dest, PieceType::rook));
            moveList.add(Move(PieceType::pawn, fromSide, from, dest, PieceType::bishop));
            moveList.add(Move(PieceType::pawn, fromSide, from, dest, PieceType::knight));
        }
    }
}

void EgtbBoardCore::clearCastleRights(int rookPos, Side rookSide) {
    switch (rookPos) {
        case 0:
            if (rookSide == Side::black) {
                castleRights[B] &= ~CASTLERIGHT_LONG;
            }
            break;
        case 7:
            if (rookSide == Side::black) {
                castleRights[B] &= ~CASTLERIGHT_SHORT;
            }
            break;
        case 56:
            if (rookSide == Side::white) {
                castleRights[W] &= ~CASTLERIGHT_LONG;
            }
            break;
        case 63:
            if (rookSide == Side::white) {
                castleRights[W] &= ~CASTLERIGHT_SHORT;
            }
            break;
    }
}


int EgtbBoardCore::findKing(Side side) const
{
    int sd = static_cast<int>(side);
    if (pieceList[sd][0].type == PieceType::king) {
        return pieceList[sd][0].idx;
    }

    for (int pos = 0; pos < 64; ++pos) {
        if (isPiece(pos, PieceType::king, side)) {
            return pos;
        }
    }
    return -1;
}


void EgtbBoardCore::genLegalOnly(MoveList& moveList, Side attackerSide, bool captureOnly) {
    gen(moveList, attackerSide, captureOnly);

    Hist hist;
    int j = 0;
    for (int i = 0; i < moveList.end; i++) {
        make(moveList.list[i], hist);
        if (!isIncheck(attackerSide)) {
            moveList.list[j] = moveList.list[i];
            j++;
        }
        takeBack(hist);
    }
    moveList.end = j;
}

bool EgtbBoardCore::isIncheck(Side beingAttackedSide) const {
    int kingPos = findKing(beingAttackedSide);
    Side attackerSide = getXSide(beingAttackedSide);
    return beAttacked(kingPos, attackerSide);
}


void EgtbBoardCore::make(const Move& move, Hist& hist) {
    auto movep = getPiece(move.from);
    auto cap = getPiece(move.dest);

    hist.enpassant = enpassant;
    hist.status = _status;
    hist.castleRights[W] = castleRights[W];
    hist.castleRights[B] = castleRights[B];
    hist.move = move;

    hist.movep = movep;
    hist.cap = cap;
    setPiece(move.dest, movep);
    setEmpty(move.from);

    assert(hist.cap.type != PieceType::king);

    enpassant = -1;

    if ((castleRights[B] + castleRights[W]) && hist.cap.type == PieceType::rook) {
        clearCastleRights(move.dest, hist.cap.side);
    }

    switch (movep.type) {
        case PieceType::king: {
            if (movep.side == Side::white) {
                castleRights[W] &= ~(CASTLERIGHT_LONG|CASTLERIGHT_SHORT);
            } else {
                castleRights[B] &= ~(CASTLERIGHT_LONG|CASTLERIGHT_SHORT);
            }

            if (abs(move.from - move.dest) == 2) { // castle
                assert(move.from == 4 || move.from == 60);
                assert(isEmpty((move.from + move.dest) / 2));
                int rookPos = move.from + (move.from < move.dest ? 3 : -4);
                assert(getPiece(rookPos).type == PieceType::rook);
                int newRookPos = (move.from + move.dest) / 2;
                setPiece(newRookPos, Piece(PieceType::rook, rookPos > 32 ? Side::white : Side::black));
                setEmpty(rookPos);
            }
            break;
        }

        case PieceType::rook: {
            if (castleRights[W] + castleRights[B]) {
                clearCastleRights(move.from, movep.side);
            }
            break;
        }

        case PieceType::pawn: {
            int d = abs(move.from - move.dest);

            if (d == 16) {
                assert(hist.cap.isEmpty());
                enpassant = (move.from + move.dest) / 2;
            } else if (move.dest == hist.enpassant) {
                if (!hist.cap.isEmpty()) {
                    std::cerr << "Wrong enpassant" << std::endl;
                }
                int ep = move.dest + (movep.side == Side::white ? +8 : -8);
                hist.cap = getPiece(ep);
                setEmpty(ep);
            } else {
                if (move.promote != PieceType::empty) {
                    assert(move.dest < 8 || move.dest >= 56);
                    setPiece(move.dest, Piece(move.promote, move.dest < 8 ? Side::white : Side::black));
                }
            }
            break;
        }
        default:
            break;
    }

    pieceList_make(hist);
    checkEnpassant();
}

void EgtbBoardCore::takeBack(const Hist& hist) {
    auto movep = getPiece(hist.move.dest);
    setPiece(hist.move.from, movep);

    int capPos = hist.move.dest;

    if (movep.type == PieceType::pawn && hist.enpassant == hist.move.dest) {
        capPos = hist.move.dest + (movep.side == Side::white ? +8 : -8);
        setEmpty(hist.move.dest);
    }
    setPiece(capPos, hist.cap);

    if (movep.type == PieceType::king) {
        if (abs(hist.move.from - hist.move.dest) == 2) {
            int rookPos = hist.move.from + (hist.move.from < hist.move.dest ? 3 : -4);
            assert(isEmpty(rookPos));
            int newRookPos = (hist.move.from + hist.move.dest) / 2;
            setPiece(rookPos, Piece(PieceType::rook, hist.move.dest < 8 ? Side::black : Side::white));
            setEmpty(newRookPos);
        }
    }

    if (hist.move.promote != PieceType::empty) {
        setPiece(hist.move.from, Piece(PieceType::pawn, hist.move.dest < 8 ? Side::white : Side::black));
    }

    _status = hist.status;
    castleRights[0] = hist.castleRights[0];
    castleRights[1] = hist.castleRights[1];
    enpassant = hist.enpassant;

    pieceList_takeback(hist);
}


void EgtbBoardCore::pieceList_reset(Piece *pieceList) {
    for(int i = 0; i < 16; i++) {
        pieceList[i].type = pieceList[16 + i].type = PieceType::empty;
    }
}

bool EgtbBoardCore::pieceList_set(Piece *pieceList, int pos, PieceType type, Side side) {
    int d = side == Side::white ? 16 : 0;

    if (type == PieceType::king) {
        pieceList[d].type = PieceType::king;
        pieceList[d].idx = pos;
        pieceList[d].side = side;
        return true;
    }

    int x = -1;
    for (int t = 1; t < 16; t++) {
        if (pieceList[d + t].idx == pos) {
            x = d + t;
            break;
        }
        if (pieceList[d + t].isEmpty() && x < 0) {
            x = d + t;
        }
    }
    if (x > 0) {
        pieceList[x].type = type;
        pieceList[x].idx = pos;
        pieceList[x].side = side;
        return true;
    }
    return false;
}

bool EgtbBoardCore::pieceList_setEmpty(Piece *pieceList, int pos, PieceType type, Side side) {
    int d = side == Side::white ? 16 : 0;
    if (type == PieceType::king) {
        pieceList[d].type = PieceType::empty;
        return true;
    }

    for (int t = 1; t < 16; t++) {
        if (pieceList[d + t].idx == pos) {
            pieceList[d + t].type = PieceType::empty;
            return true;
        }
    }
    return false;
}

bool EgtbBoardCore::pieceList_setEmpty(Piece *pieceList, int pos) {
    for (int sd = 0; sd < 2; sd ++) {
        if (pieceList_setEmpty(pieceList, pos, sd)) {
            return true;
        }
    }

    return false;
}

bool EgtbBoardCore::pieceList_setEmpty(Piece *pieceList, int pos, int sd) {
    int d = sd == 0 ? 0 : 16;
    for(int i = 0; i < 16; i++) {
        if (pieceList[i + d].idx == pos) {
            pieceList[i + d].type = PieceType::empty;
            return true;
        }
    }
    return false;
}

int EgtbBoardCore::pieceList_countStrong(const Piece *pieceList, Side side) {
    auto p = pieceList + (side == Side::white ? 16 : 0);

    int cnt = 0;
    for(int i = 1; i < 16; i ++) {
        if (static_cast<int>(p[i].type) < static_cast<int>(PieceType::pawn)) cnt++;
    }
    return cnt;
}

bool EgtbBoardCore::pieceList_isDraw(const Piece *pieceList) {
    for(int i = 1; i < 16; i ++) {
        if (pieceList[i].type != PieceType::empty || pieceList[i + 16].type != PieceType::empty) {
            return false;
        }
    }
    return true;
}

bool EgtbBoardCore::pieceList_make(const Hist& hist) {
    if (!hist.cap.isEmpty()) {
        bool ok = false;

        int capPos = hist.move.dest;
        if (hist.enpassant == capPos) {
            capPos += capPos < 32 ? +8 : -8;
        }
        for (int t = 0, sd = static_cast<int>(hist.cap.side); t < 16; t++) {
            if (pieceList[sd][t].idx == capPos && pieceList[sd][t].type != PieceType::empty) {
                pieceList[sd][t].type = PieceType::empty;
                ok = true;
                break;
            }
        }
        if (!ok) {
            return false;
        }
    }
    for (int t = 0, sd = static_cast<int>(hist.move.side); t < 16; t++) {
        if (pieceList[sd][t].idx == hist.move.from && pieceList[sd][t].type != PieceType::empty) {
            pieceList[sd][t].idx = hist.move.dest;

            if (hist.move.promote != PieceType::empty) {
                pieceList[sd][t].type = hist.move.promote;
            }
            return true;
        }
    }
    return false;
}

bool EgtbBoardCore::pieceList_takeback(const Hist& hist) {
    bool ok = false;
    for (int t = 0, sd = static_cast<int>(hist.move.side); t < 16; t++) {
        if (pieceList[sd][t].idx == hist.move.dest && pieceList[sd][t].type != PieceType::empty) {
            pieceList[sd][t].idx = hist.move.from;
            if (hist.move.promote != PieceType::empty) {
                pieceList[sd][t].type = PieceType::pawn;
            }
            ok = true;
            break;
        }
    }
    if (!ok) {
        return false;
    }
    if (hist.cap.isEmpty()) {
        return true;
    }
    for (int t = 0, sd = static_cast<int>(hist.cap.side); t < 16; t++) {
        if (pieceList[sd][t].type == PieceType::empty) {
            pieceList[sd][t] = hist.cap;

            pieceList[sd][t].idx = hist.move.dest;

            if (hist.enpassant == hist.move.dest) {
                assert(hist.move.type == PieceType::pawn && hist.cap.type == PieceType::pawn);
                pieceList[sd][t].idx += hist.move.dest > 32 ? -8 : +8;
            }

            return true;
        }
    }
    return false;
}

void EgtbBoardCore::pieceList_createList(Piece *pieceList) const {
    pieceList_reset(pieceList);

    for(int pos = 0; pos < 90; pos++) {
        auto piece = getPiece(pos);
        if (piece.isEmpty()) {
            continue;
        }
        pieceList_set(pieceList, pos, piece.type, piece.side);
    }
}

bool EgtbBoardCore::pieceList_setupBoard(const Piece *thePieceList) {
    reset();

    if (thePieceList) {
        memcpy(pieceList, thePieceList, sizeof(pieceList));
    }

    for (int sd = 0; sd < 2; sd++) {
        for(int i = 0; i < 16; i++) {
            auto p = pieceList[sd][i];
            if (!p.isEmpty()) {
                if (isEmpty(p.idx)) {
                    setPiece(p.idx, p);
                } else {
                    return false;
                }
            }
        }
    }

    return true;
}


Side EgtbBoardCore::strongSide(const Piece *pieceList) {
    int mat[] = { 0, 0};
    for (int sd = 0, d = 0; sd < 2; sd++, d = 16) {
        for(int i = 1; i < 16; i++) {
            auto p = pieceList[d + i];
            if (!p.isEmpty()) {
                mat[sd] += exchangePieceValue[static_cast<int>(p.type)];
            }
        }
    }

    return mat[W] > mat[B] ? Side::white : Side::black;
}


static const int flip_h[64] = {
    7, 6, 5, 4, 3, 2, 1, 0,
    15,14,13,12,11,10, 9, 8,
    23,22,21,20,19,18,17,16,
    31,30,29,28,27,26,25,24,
    39,38,37,36,35,34,33,32,
    47,46,45,44,43,42,41,40,
    55,54,53,52,51,50,49,48,
    63,62,61,60,59,58,57,56
};

static const int flip_v[64] = {
    56,57,58,59,60,61,62,63,
    48,49,50,51, 52,53,54,55,
    40,41,42,43, 44,45,46,47,
    32,33,34,35, 36,37,38,39,

    24,25,26,27, 28,29,30,31,
    16,17,18,19, 20,21,22,23,
    8, 9,10,11, 12,13,14,15,
    0, 1, 2, 3,  4, 5, 6, 7
};

static const int flip_r90[64] = {
    7,15,23,31,39,47,55,63,
    6,14,22,30,38,46,54,62,
    5,13,21,29,37,45,53,61,
    4,12,20,28,36,44,52,60,
    3,11,19,27,35,43,51,59,
    2,10,18,26,34,42,50,58,
    1, 9,17,25,33,41,49,57,
    0, 8,16,24,32,40,48,56
};

static const int flip_r270[64] = {
    56,48,40,32,24,16, 8, 0,
    57,49,41,33,25,17, 9, 1,
    58,50,42,34,26,18,10, 2,
    59,51,43,35,27,19,11, 3,
    60,52,44,36,28,20,12, 4,
    61,53,45,37,29,21,13, 5,
    62,54,46,38,30,22,14, 6,
    63,55,47,39,31,23,15, 7
};

static const int flip_vh[64] = { // a8-h1
    0, 8,16,24,32,40,48,56,
    1, 9,17,25,33,41,49,57,
    2,10,18,26,34,42,50,58,
    3,11,19,27,35,43,51,59,
    4,12,20,28,36,44,52,60,
    5,13,21,29,37,45,53,61,
    6,14,22,30,38,46,54,62,
    7,15,23,31,39,47,55,63
};

static const int flip_hv[64] = { // a1-h8
    63,55,47,39,31,23,15, 7,
    62,54,46,38,30,22,14, 6,
    61,53,45,37,29,21,13, 5,
    60,52,44,36,28,20,12, 4,
    59,51,43,35,27,19,11, 3,
    58,50,42,34,26,18,10, 2,
    57,49,41,33,25,17, 9, 1,
    56,48,40,32,24,16, 8, 0
};

int EgtbBoardCore::flip(int pos, FlipMode flipMode) {
    switch (flipMode) {
        case FlipMode::none: return pos;
        case FlipMode::horizontal: return flip_h[pos];
        case FlipMode::vertical: return flip_v[pos];
        case FlipMode::rotate180: return 63 - pos;    // around

        case FlipMode::flipVH: return flip_vh[pos];
        case FlipMode::flipHV: return flip_hv[pos];
        case FlipMode::rotate90: return flip_r90[pos];
        case FlipMode::rotate270: return flip_r270[pos];

        default:
            assert(false);
    }
    return 0;
}

void EgtbBoardCore::flip(FlipMode flipMode) {
    Piece bkPieceList[2][16];
    memcpy(bkPieceList, pieceList, sizeof(bkPieceList));

    reset();
    pieceList_reset((Piece *)pieceList);

    for(int sd = 0; sd < 2; sd++) {
        //        Side side = static_cast<Side>(sd);
        for(int i = 0; i < 16; i++) {
            auto p = bkPieceList[sd][i];
            if (p.isEmpty()) {
                continue;
            }
            int newpos = flip(p.idx, flipMode);
            setPiece(newpos, p);
        }
    }
}

static const FlipMode flipflip_h[] = { FlipMode::horizontal, FlipMode::none, FlipMode::rotate180, FlipMode::rotate90, FlipMode::rotate270, FlipMode::flipHV, FlipMode::vertical, FlipMode::flipVH };
static const FlipMode flipflip_v[] = { FlipMode::vertical, FlipMode::rotate180, FlipMode::none, FlipMode::rotate90, FlipMode::rotate270, FlipMode::flipVH, FlipMode::horizontal, FlipMode::flipHV };
static const FlipMode flipflip_vh[] = { FlipMode::flipVH, FlipMode::rotate270, FlipMode::rotate90, FlipMode::none, FlipMode::rotate180, FlipMode::vertical, FlipMode::flipHV, FlipMode::horizontal };
static const FlipMode flipflip_hv[] = { FlipMode::flipHV, FlipMode::rotate90, FlipMode::rotate270, FlipMode::rotate180, FlipMode::none, FlipMode::horizontal, FlipMode::flipVH, FlipMode::vertical};
static const FlipMode flipflip_r90[] = { FlipMode::rotate90, FlipMode::flipHV, FlipMode::flipVH, FlipMode::horizontal, FlipMode::vertical, FlipMode::rotate180, FlipMode::rotate270, FlipMode::none };
static const FlipMode flipflip_r180[] = { FlipMode::rotate180, FlipMode::vertical, FlipMode::horizontal, FlipMode::flipHV, FlipMode::flipVH, FlipMode::rotate270, FlipMode::none, FlipMode::rotate90 };
static const FlipMode flipflip_r270[] = { FlipMode::rotate270, FlipMode::flipVH, FlipMode::flipHV, FlipMode::vertical, FlipMode::horizontal, FlipMode::none, FlipMode::rotate90, FlipMode::rotate180 };

FlipMode EgtbBoardCore::flip(FlipMode oMode, FlipMode flipMode) {
    switch (flipMode) {
        case FlipMode::none:
            break;

        case FlipMode::horizontal:
            return flipflip_h[static_cast<int>(oMode)];

        case FlipMode::vertical:
            return flipflip_v[static_cast<int>(oMode)];

        case FlipMode::flipVH:
            return flipflip_vh[static_cast<int>(oMode)];

        case FlipMode::flipHV:
            return flipflip_hv[static_cast<int>(oMode)];

        case FlipMode::rotate90:
            return flipflip_r90[static_cast<int>(oMode)];

        case FlipMode::rotate180:
            return flipflip_r180[static_cast<int>(oMode)];

        case FlipMode::rotate270:
            return flipflip_r270[static_cast<int>(oMode)];
    }
    return oMode;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void EgtbBoard::gen(MoveList& moves, Side side, bool captureOnly) const {
    assert(isValid());
    for (int pos = 0; pos < 64; ++pos) {
        auto piece = pieces[pos];

        if (piece.side != side) {
            continue;
        }

        switch (piece.type) {
            case PieceType::king:
            {
                int col = COL(pos);
                if (col) { // go left
                    gen_addMove(moves, pos, pos - 1, captureOnly);
                }
                if (col < 7) { // right
                    gen_addMove(moves, pos, pos + 1, captureOnly);
                }
                if (pos > 7) { // up
                    gen_addMove(moves, pos, pos - 8, captureOnly);
                }
                if (pos < 56) { // down
                    gen_addMove(moves, pos, pos + 8, captureOnly);
                }

                if (col && pos > 7) { // left up
                    gen_addMove(moves, pos, pos - 9, captureOnly);
                }
                if (col < 7 && pos > 7) { // right up
                    gen_addMove(moves, pos, pos - 7, captureOnly);
                }
                if (col && pos < 56) { // left down
                    gen_addMove(moves, pos, pos + 7, captureOnly);
                }
                if (col < 7 && pos < 56) { // right down
                    gen_addMove(moves, pos, pos + 9, captureOnly);
                }

                if (captureOnly) {
                    break;
                }
                if ((pos ==  4 && castleRights[B]) ||
                    (pos == 60 && castleRights[W])) {
                    if (pos == 4) {
                        if ((castleRights[B] & CASTLERIGHT_LONG) &&
                            pieces[1].isEmpty() && pieces[2].isEmpty() &&pieces[3].isEmpty() &&
                            !beAttacked(2, Side::white) && !beAttacked(3, Side::white)) {
                            assert(isPiece(0, PieceType::rook, Side::black));
                            gen_addMove(moves, 4, 2, captureOnly);
                        }
                        if ((castleRights[B] & CASTLERIGHT_SHORT) &&
                            pieces[5].isEmpty() && pieces[6].isEmpty() &&
                            !beAttacked(5, Side::white) && !beAttacked(6, Side::white)) {
                            assert(isPiece(7, PieceType::rook, Side::black));
                            gen_addMove(moves, 4, 6, captureOnly);
                        }
                    } else {
                        if ((castleRights[W] & CASTLERIGHT_LONG) &&
                            pieces[57].isEmpty() && pieces[58].isEmpty() && pieces[59].isEmpty() &&
                            !beAttacked(58, Side::black) && !beAttacked(59, Side::black)) {
                            assert(isPiece(56, PieceType::rook, Side::white));
                            gen_addMove(moves, 60, 58, captureOnly);
                        }
                        if ((castleRights[W] & CASTLERIGHT_SHORT) &&
                            pieces[61].isEmpty() && pieces[62].isEmpty() &&
                            !beAttacked(61, Side::black) && !beAttacked(62, Side::black)) {
                            assert(isPiece(63, PieceType::rook, Side::white));
                            gen_addMove(moves, 60, 62, captureOnly);
                        }
                    }
                }
                break;
            }

            case PieceType::queen:
            case PieceType::bishop:
            {
                for (int y = pos - 9; y >= 0 && COL(y) != 7; y -= 9) {        /* go left up */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }
                for (int y = pos - 7; y >= 0 && COL(y) != 0; y -= 7) {        /* go right up */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }
                for (int y = pos + 9; y < 64 && COL(y) != 0; y += 9) {        /* go right down */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }
                for (int y = pos + 7; y < 64 && COL(y) != 7; y += 7) {        /* go right down */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }
                /*
                 * if the piece is a bishop, stop heere, otherwise queen will be continued as a rook
                 */
                if (piece.type == PieceType::bishop) {
                    break;
                }
                // Queen continues
            }

            case PieceType::rook: // both queen and rook here
            {
                int col = COL(pos);
                for (int y=pos - 1; y >= pos - col; y--) { /* go left */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }

                for (int y=pos + 1; y < pos - col + 8; y++) { /* go right */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }

                for (int y=pos - 8; y >= 0; y -= 8) { /* go up */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }

                for (int y=pos + 8; y < 64; y += 8) { /* go down */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }

                }
                break;
            }

            case PieceType::knight:
            {
                int col = COL(pos);
                int y = pos - 6;
                if (y >= 0 && col < 6)
                    gen_addMove(moves, pos, y, captureOnly);
                y = pos - 10;
                if (y >= 0 && col > 1)
                    gen_addMove(moves, pos, y, captureOnly);
                y = pos - 15;
                if (y >= 0 && col < 7)
                    gen_addMove(moves, pos, y, captureOnly);
                y = pos - 17;
                if (y >= 0 && col > 0)
                    gen_addMove(moves, pos, y, captureOnly);
                y = pos + 6;
                if (y < 64 && col > 1)
                    gen_addMove(moves, pos, y, captureOnly);
                y = pos + 10;
                if (y < 64 && col < 6)
                    gen_addMove(moves, pos, y, captureOnly);
                y = pos + 15;
                if (y < 64 && col > 0)
                    gen_addMove(moves, pos, y, captureOnly);
                y = pos + 17;
                if (y < 64 && col < 7)
                    gen_addMove(moves, pos, y, captureOnly);
                break;
            }


            case PieceType::pawn:
            {
                int col = COL(pos);
                if (side == Side::black) {
                    if (!captureOnly && isEmpty(pos + 8)) {
                        gen_addPawnMove(moves, pos, pos + 8, captureOnly);
                    }
                    if (!captureOnly && pos < 16 && isEmpty(pos + 8) && isEmpty(pos + 16)) {
                        gen_addMove(moves, pos, pos + 16, captureOnly);
                    }

                    if (col && (getPiece(pos + 7).side == Side::white || (pos + 7 == enpassant && getPiece(pos + 7).side == Side::none))) {
                        gen_addPawnMove(moves, pos, pos + 7, captureOnly);
                    }
                    if (col < 7 && (getPiece(pos + 9).side == Side::white || (pos + 9 == enpassant && getPiece(pos + 9).side == Side::none))) {
                        gen_addPawnMove(moves, pos, pos + 9, captureOnly);
                    }
                } else {
                    if (!captureOnly && isEmpty(pos - 8)) {
                        gen_addPawnMove(moves, pos, pos - 8, captureOnly);
                    }
                    if (!captureOnly && pos >= 48 && isEmpty(pos - 8) && isEmpty(pos - 16)) {
                        gen_addMove(moves, pos, pos - 16, captureOnly);
                    }

                    if (col < 7 && (getPiece(pos - 7).side == Side::black || (pos - 7 == enpassant && getPiece(pos - 7).side == Side::none)))
                        gen_addPawnMove(moves, pos, pos - 7, captureOnly);
                    if (col && (getPiece(pos - 9).side == Side::black || (pos - 9 == enpassant && getPiece(pos - 9).side == Side::none)))
                        gen_addPawnMove(moves, pos, pos - 9, captureOnly);
                }
                break;
            }

            default:
                break;
        }
    }
}


bool EgtbBoard::beAttacked(int pos, Side attackerSide) const
{
    int row = ROW(pos), col = COL(pos);
    /* Check attacking of Knight */
    if (col > 0 && row > 1 && isPiece(pos - 17, PieceType::knight, attackerSide))
        return true;
    if (col < 7 && row > 1 && isPiece(pos - 15, PieceType::knight, attackerSide))
        return true;
    if (col > 1 && row > 0 && isPiece(pos - 10, PieceType::knight, attackerSide))
        return true;
    if (col < 6 && row > 0 && isPiece(pos - 6, PieceType::knight, attackerSide))
        return true;
    if (col > 1 && row < 7 && isPiece(pos + 6, PieceType::knight, attackerSide))
        return true;
    if (col < 6 && row < 7 && isPiece(pos + 10, PieceType::knight, attackerSide))
        return true;
    if (col > 0 && row < 6 && isPiece(pos + 15, PieceType::knight, attackerSide))
        return true;
    if (col < 7 && row < 6 && isPiece(pos + 17, PieceType::knight, attackerSide))
        return true;


    /* Check horizontal and vertical lines for attacking of Queen, Rook, King */
    /* go down */
    for (int y = pos + 8; y < 64; y += 8) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::rook ||
                    (piece.type == PieceType::king && y == pos + 8)) {
                    return true;
                }
            }
            break;
        }
    }

    /* go up */
    for (int y = pos - 8; y >= 0; y -= 8) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::rook ||
                    (piece.type == PieceType::king && y == pos - 8)) {
                    return true;
                }
            }
            break;
        }
    }

    /* go left */
    for (int y = pos - 1; y >= pos - col; y--) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::rook ||
                    (piece.type == PieceType::king && y == pos - 1)) {
                    return true;
                }
            }
            break;
        }
    }

    /* go right */
    for (int y = pos + 1; y < pos - col + 8; y++) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::rook ||
                    (piece.type == PieceType::king && y == pos + 1)) {
                    return true;
                }
            }
            break;
        }
    }

    /* Check diagonal lines for attacking of Queen, Bishop, King, Pawn */
    /* go right down */
    for (int y = pos + 9; y < 64 && COL(y) != 0; y += 9) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::bishop ||
                    (y == pos + 9 && (piece.type == PieceType::king || (piece.type == PieceType::pawn && piece.side == Side::white)))) {
                    return true;
                }
            }
            break;
        }
    }

    /* go left down */
    for (int y = pos + 7; y < 64 && COL(y) != 7; y += 7) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::bishop ||
                    (y == pos + 7 && (piece.type == PieceType::king || (piece.type == PieceType::pawn && piece.side == Side::white)))) {
                    return true;
                }
            }
            break;
        }
    }

    /* go left up */
    for (int y = pos - 9; y >= 0 && COL(y) != 7; y -= 9) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::bishop ||
                    (y == pos - 9 && (piece.type == PieceType::king || (piece.type == PieceType::pawn && piece.side == Side::black)))) {
                    return true;
                }
            }
            break;
        }
    }

    /* go right up */
    for (int y = pos - 7; y >= 0 && COL(y) != 0; y -= 7) {
        auto piece = getPiece(y);
        if (!piece.isEmpty()) {
            if (piece.side == attackerSide) {
                if (piece.type == PieceType::queen || piece.type == PieceType::bishop ||
                    (y == pos - 7 && (piece.type == PieceType::king || (piece.type == PieceType::pawn && piece.side == Side::black)))) {
                    return true;
                }
            }
            break;
        }
    }

    return false;
}


void EgtbBoard::make(const Move& move, Hist& hist) {
    hist.enpassant = enpassant;
    hist.status = _status;
    hist.castleRights[0] = castleRights[0];
    hist.castleRights[1] = castleRights[1];
    hist.move = move;
    hist.cap = pieces[move.dest];

    auto p = pieces[move.from];
    pieces[move.dest] = p;
    pieces[move.from].setEmpty();

    enpassant = -1;

    if ((castleRights[0] + castleRights[1]) && hist.cap.type == PieceType::rook) {
        clearCastleRights(move.dest, hist.cap.side);
    }

    switch (p.type) {
        case PieceType::king: {
            if (p.side == Side::white) {
                castleRights[0] &= ~(CASTLERIGHT_LONG|CASTLERIGHT_SHORT);
            } else {
                castleRights[1] &= ~(CASTLERIGHT_LONG|CASTLERIGHT_SHORT);
            }

            if (abs(move.from - move.dest) == 2) { // castle
                int rookPos = move.from + (move.from < move.dest ? 3 : -4);
                int newRookPos = (move.from + move.dest) / 2;
                pieces[newRookPos] = pieces[rookPos];
                pieces[rookPos].setEmpty();
            }
            break;
        }

        case PieceType::rook: {
            if (castleRights[0] + castleRights[1]) {
                clearCastleRights(move.from, p.side);
            }
            break;
        }

        case PieceType::pawn: {
            int d = abs(move.from - move.dest);

            if (d == 16) {
                enpassant = (move.from + move.dest) / 2;
            } else if (move.dest == hist.enpassant) {
                int ep = move.dest + (p.side == Side::white ? +8 : -8);
                hist.cap = pieces[ep];
                pieces[ep].setEmpty();
            } else {
                if (move.promote != PieceType::empty) {
                    pieces[move.dest].type = move.promote;
                }
            }
            break;
        }
        default:
            break;
    }



    auto b = pieceList_make(hist);
    assert(b);
}

void EgtbBoard::takeBack(const Hist& hist) {
    pieces[hist.move.from] = pieces[hist.move.dest];

    int capPos = hist.move.dest;

    if (pieces[hist.move.from].type == PieceType::pawn && hist.enpassant == hist.move.dest) {
        capPos = hist.move.dest + (pieces[hist.move.from].side == Side::white ? +8 : -8);
        pieces[hist.move.dest].setEmpty();
    }
    pieces[capPos] = hist.cap;

    if (pieces[hist.move.from].type == PieceType::king) {
        if (abs(hist.move.from - hist.move.dest) == 2) {
            int rookPos = hist.move.from + (hist.move.from < hist.move.dest ? 3 : -4);
            int newRookPos = (hist.move.from + hist.move.dest) / 2;
            pieces[rookPos] = pieces[newRookPos];
            pieces[newRookPos].setEmpty();
        }
    }

    if (hist.move.promote != PieceType::empty) {
        pieces[hist.move.from].type = PieceType::pawn;
    }

    _status = hist.status;
    castleRights[0] = hist.castleRights[0];
    castleRights[1] = hist.castleRights[1];
    enpassant = hist.enpassant;

    pieceList_takeback(hist);
    assert(isValid());
}

bool EgtbBoard::setup(const std::vector<Piece> pieceVec, Side _side, Squares _enpassant) {
    pieceList_reset((Piece *)pieceList);
    reset();

    side = _side;
    for (auto && p : pieceVec) {
        if (p.isEmpty()) {
            continue;
        }

        if (p.idx < 0 || p.idx >= 64 || !isEmpty(p.idx)) {
            return false;
        }
        setPiece(p.idx, p);
        pieceList_set((Piece *)pieceList, p.idx, p.type, p.side);
    }

    enpassant = static_cast<int>(_enpassant);
    checkEnpassant();
    return true;
}

