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

#include "Egtb.h"
#include "EgtbKey.h"

namespace egtb {
    EgtbKey egtbKey;
} // namespace


using namespace egtb;

extern const int tb_kIdxToPos[10];
extern int *kk_2, *kk_8;

static const int tb_flipMode[64] = {
    0, 0, 0, 0, 1, 1, 1, 1,
    3, 0, 0, 0, 1, 1, 1, 7,
    3, 3, 0, 0, 1, 1, 7, 7,
    3, 3, 3, 0, 1, 7, 7, 7,
    5, 5, 5, 2, 6, 4, 4, 4,
    5, 5, 2, 2, 6, 6, 4, 4,
    5, 2, 2, 2, 6, 6, 6, 4,
    2, 2, 2, 2, 6, 6, 6, 6
};

static const int tb_kIdx[64] = {
    0, 1, 2, 3, -1,-1,-1,-1,
    -1, 4, 5, 6, -1,-1,-1,-1,
    -1,-1, 7, 8, -1,-1,-1,-1,
    -1,-1,-1, 9, -1,-1,-1,-1,

    -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1
};

static int* tb_xx, *tb_xxx, *tb_xxxx;
static int* tb_pp, *tb_ppp, *tb_pppp;
int *kk_2, *kk_8;

const int tb_kIdxToPos[10] = {
    0, 1, 2, 3, 9, 10, 11, 18, 19, 27
};

static int bSearch(const int* array, int sz, int key) {
    int i = 0, j = sz - 1;

    while (i <= j) {
        int idx = (i + j) / 2;
        if (key == array[idx]) {
            return idx;
        }
        if (key < array[idx]) j = idx - 1;
        else i = idx + 1;
    }

    return -1;
}

static void sort_tbkeys(int* tbkeys, int sz) {
    std::qsort(tbkeys, sz, sizeof(int), [](const void* a, const void* b) {
        const int* x = static_cast<const int*>(a);
        const int* y = static_cast<const int*>(b);
        return (int)(*x - *y);
    });
}

void EgtbKey::createKingKeys() {
    kk_8 = new int[EGTB_SIZE_KK8];
    int x = 0;

    for(int i = 0; i < sizeof(tb_kIdxToPos) / sizeof(int); i++) {
        int k0 = tb_kIdxToPos[i];
        int r0 = ROW(k0), f0 = COL(k0);
        for(int k1 = 0; k1 < 64; k1++) {
            if (k0 == k1 || (abs(ROW(k1) - r0) <= 1 && abs(COL(k1) - f0) <= 1)) {
                continue;
            }

            kk_8[x++] = k0 << 8 | k1;
        }
    }

    kk_2 = new int[EGTB_SIZE_KK2];
    x = 0;

    for(int k0 = 0; k0 < 64; k0++) {
        int f0 = COL(k0);
        if (f0 > 3) {
            continue;
        }
        int r0 = ROW(k0);

        for(int k1 = 0; k1 < 64; k1++) {
            if (k0 == k1 || (abs(ROW(k1) - r0) <= 1 && abs(COL(k1) - f0) <= 1)) {
                continue;
            }

            kk_2[x++] = k0 << 8 | k1;
        }
    }
}

void EgtbKey::createXXKeys() {
    tb_xx = new int[EGTB_SIZE_XX];
    tb_xxx = new int[EGTB_SIZE_XXX];
    tb_xxxx = new int[EGTB_SIZE_XXXX];

    int k0 = 0, k1 = 0, k2 = 0;

    for(int i0 = 0; i0 < 64; i0++) {
        for(int i1 = i0 + 1; i1 < 64; i1++) {
            tb_xx[k0++] = i0 << 8 | i1;
            for(int i2 = i1 + 1; i2 < 64; i2++) {
                tb_xxx[k1++] = i0 << 16 | i1 << 8 | i2;
                for(int i3 = i2 + 1; i3 < 64; i3++) {
                    tb_xxxx[k2++] = i0 << 24 | i1 << 16 | i2 << 8 | i3;
                }
            }
        }
    }

    // Pawns
    tb_pp = new int[EGTB_SIZE_PP];
    tb_ppp = new int[EGTB_SIZE_PPP];
    tb_pppp = new int[EGTB_SIZE_PPPP];

    k0 = k1 = k2 = 0;

    for(int i0 = 8; i0 < 56; i0++) {
        for(int i1 = i0 + 1; i1 < 56; i1++) {
            tb_pp[k0++] = i0 << 8 | i1;
            for(int i2 = i1 + 1; i2 < 56; i2++) {
                tb_ppp[k1++] = i0 << 16 | i1 << 8 | i2;
                for(int i3 = i2 + 1; i3 < 56; i3++) {
                    tb_pppp[k2++] = i0 << 24 | i1 << 16 | i2 << 8 | i3;
                }
            }
        }
    }
}

int EgtbKey::getKey_x(int pos0)
{
    return pos0;
}

int EgtbKey::getKey_xx(int pos0, int pos1)
{
    auto p0 = MIN(pos0, pos1);
    auto p1 = MAX(pos0, pos1);

    int x = p0 << 8 | p1;

    return bSearch(tb_xx, EGTB_SIZE_XX, x);
}

int EgtbKey::getKey_xxx(int pos0, int pos1, int pos2)
{
    int p[3] = { pos0, pos1, pos2 };
    sort_tbkeys(p, 3);

    int x = p[0] << 16 | p[1] << 8 | p[2];
    return bSearch(tb_xxx, EGTB_SIZE_XXX, x);
}

int EgtbKey::getKey_xxxx(int pos0, int pos1, int pos2, int pos3)
{
    int p[4] = { pos0, pos1, pos2, pos3 };
    sort_tbkeys(p, 4);

    int x = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    return bSearch(tb_xxxx, EGTB_SIZE_XXXX, x);
}


int EgtbKey::getKey_p(int pos0)
{
    assert(pos0 >= 8 && pos0 < 56);
    return pos0 - 8;
}

int EgtbKey::getKey_pp(int pos0, int pos1)
{
    auto p0 = MIN(pos0, pos1);
    auto p1 = MAX(pos0, pos1);

    int x = p0 << 8 | p1;
    return bSearch(tb_pp, EGTB_SIZE_PP, x);
}

int EgtbKey::getKey_ppp(int pos0, int pos1, int pos2)
{
    int p[3] = { pos0, pos1, pos2 };
    sort_tbkeys(p, 3);
    int x = p[0] << 16 | p[1] << 8 | p[2];
    return bSearch(tb_ppp, EGTB_SIZE_PPP, x);
}

int EgtbKey::getKey_pppp(int pos0, int pos1, int pos2, int pos3)
{
    int p[4] = { pos0, pos1, pos2, pos3 };
    sort_tbkeys(p, 4);

    int x = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    return bSearch(tb_pppp, EGTB_SIZE_PPPP, x);
}

bool EgtbKey::setupBoard_x(EgtbBoardCore& board, int key, PieceType type, Side side) const
{
    if (type == PieceType::pawn) {
        key += 8;
    }

    auto sd = static_cast<int>(side);

    for(int i = 1; i < 16; i++) {
        if (board.pieceList[sd][i].isEmpty()) {
            board.pieceList[sd][i].type = type;
            board.pieceList[sd][i].side = side;
            board.pieceList[sd][i].idx = key;
            return true;
        }
    }
    return false;
}

bool EgtbKey::setupBoard_xx(EgtbBoardCore& board, int key, PieceType type, Side side) const
{
    int xx;

    if (type != PieceType::pawn) {
        xx = tb_xx[key];
    } else {
        assert(key < EGTB_SIZE_PP);
        xx = tb_pp[key];
    }

    int pos0 = xx >> 8, pos1 = xx & 0xff;
    auto sd = static_cast<int>(side);

    for(int i = 1; i < 16; i++) {
        if (board.pieceList[sd][i].isEmpty()) {
            board.pieceList[sd][i].type = type;
            board.pieceList[sd][i].side = side;
            board.pieceList[sd][i].idx = pos0;

            for(i++; i < 16; i++) {
                if (board.pieceList[sd][i].isEmpty()) {
                    board.pieceList[sd][i].type = type;
                    board.pieceList[sd][i].side = side;
                    board.pieceList[sd][i].idx = pos1;
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool EgtbKey::setupBoard_xxx(EgtbBoardCore& board, int key, PieceType type, Side side) const
{
    int xx;
    if (type != PieceType::pawn) {
        xx = tb_xxx[key];
    } else {
        assert(key >= 0 && key < EGTB_SIZE_PPP);
        xx = tb_ppp[key];
    }
    int pos0 = xx >> 16, pos1 = (xx >> 8) & 0xff, pos2 = xx & 0xff;
    auto sd = static_cast<int>(side);

    for(int i = 1; i < 16; i++) {
        if (board.pieceList[sd][i].isEmpty()) {
            board.pieceList[sd][i].type = type;
            board.pieceList[sd][i].side = side;
            board.pieceList[sd][i].idx = pos0;

            for(i++; i < 16; i++) {
                if (board.pieceList[sd][i].isEmpty()) {
                    board.pieceList[sd][i].type = type;
                    board.pieceList[sd][i].side = side;
                    board.pieceList[sd][i].idx = pos1;

                    for(i++; i < 16; i++) {
                        if (board.pieceList[sd][i].isEmpty()) {
                            board.pieceList[sd][i].type = type;
                            board.pieceList[sd][i].side = side;
                            board.pieceList[sd][i].idx = pos2;
                            return true;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    return false;
}

bool EgtbKey::setupBoard_xxxx(EgtbBoardCore& board, int key, PieceType type, Side side) const
{
    int xx;
    if (type != PieceType::pawn) {
        xx = tb_xxxx[key];
    } else {
        assert(key >= 0 && key < EGTB_SIZE_PPPP);
        xx = tb_pppp[key];
    }

    int pos0 = xx >> 24, pos1 = (xx >> 16) & 0xff, pos2 = (xx >> 8) & 0xff, pos3 = xx & 0xff;
    auto sd = static_cast<int>(side);

    for(int i = 1; i < 16; i++) {
        if (board.pieceList[sd][i].isEmpty()) {
            board.pieceList[sd][i].type = type;
            board.pieceList[sd][i].side = side;
            board.pieceList[sd][i].idx = pos0;

            for(i++; i < 16; i++) {
                if (board.pieceList[sd][i].isEmpty()) {
                    board.pieceList[sd][i].type = type;
                    board.pieceList[sd][i].side = side;
                    board.pieceList[sd][i].idx = pos1;

                    for(i++; i < 16; i++) {
                        if (board.pieceList[sd][i].isEmpty()) {
                            board.pieceList[sd][i].type = type;
                            board.pieceList[sd][i].side = side;
                            board.pieceList[sd][i].idx = pos2;

                            for(i++; i < 16; i++) {
                                if (board.pieceList[sd][i].isEmpty()) {
                                    board.pieceList[sd][i].type = type;
                                    board.pieceList[sd][i].side = side;
                                    board.pieceList[sd][i].idx = pos3;
                                    return true;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    return false;
}

void EgtbKey::initOnce() {
    createKingKeys();
    createXXKeys();
}

EgtbKey::EgtbKey() {
    initOnce();
}

void EgtbKey::getKey(EgtbKeyRec& rec, const EgtbBoardCore& board, const int* idxArr, const i64* idxMult, u32 order) {
    int sd = W;

    // Check which side for left hand side
    int mat[] = { 0, 0 };
    int cnt[] = { 0, 0 };
    int pawnCnt = 0;

    for (int s = 0, d = 0; s < 2; s++, d = 16) {
        for(int i = 1; i < 16; i++) {
            if (!board.pieceList[s][i].isEmpty()) {
                cnt[s]++;
                int type = static_cast<int>(board.pieceList[s][i].type);
                mat[s] += exchangePieceValue[type];
                if (board.pieceList[s][i].type == PieceType::pawn) {
                    pawnCnt++;
                }
            }
        }
    }

    auto flipMode = FlipMode::none;
    if (cnt[B] > cnt[W] || (cnt[B] == cnt[W] && mat[B] > mat[W])) {
        sd = B;
        flipMode = FlipMode::vertical;
    }

    rec.flipSide = sd == B;

    if (!order) {
        order = 0 | 1 << 3 | 2 << 6 | 3 << 9 | 4 << 12 | 5 << 15;
    }

    u32 o[6] = {
        order & 0x7, (order >> 3) & 0x7, (order >> 6) & 0x7, (order >> 9) & 0x7, (order >> 12) & 0x7, (order >> 15) & 0x7
    };

    i64 key = 0;

    for(int i = 0, stdSd = W; idxArr[i] != EGTB_IDX_NONE; i++) {
        int j = o[i];
        auto attr = idxArr[j];
        auto mul = idxMult[j];

        if (stdSd != (attr >> 8)) {
            stdSd = (attr >> 8);
            sd = 1 - sd;
        }

        attr &= 0xff;
        switch (attr) {
            case EGTB_IDX_K_8:
            {
                auto idx = board.pieceList[sd][0].idx;

                int flip = tb_flipMode[idx];
                flipMode = EgtbBoardCore::flip(flipMode, static_cast<FlipMode>(flip));
                idx = EgtbBoardCore::flip(idx, flipMode);

                auto idx2 = tb_kIdx[idx]; assert(idx2 >= 0 && idx2 < 10);
                key += idx2 * mul;
                break;
            }

            case EGTB_IDX_K_2:
            {
                int pos = board.pieceList[sd][0].idx;
                pos = EgtbBoardCore::flip(pos, flipMode);
                auto f = pos & 0x7;
                if (f > 3) {
                    flipMode = EgtbBoardCore::flip(flipMode, FlipMode::horizontal);
                    f = 7 - f;
                }
                auto r = pos >> 3;
                int idx = (r << 2) + f;
                assert(idx >= 0 && idx < 32);
                key += idx * mul;
                break;
            }

            case EGTB_IDX_KK_2:
            {
                int pos0 = EgtbBoardCore::flip(board.pieceList[sd][0].idx, flipMode);
                int pos1 = EgtbBoardCore::flip(board.pieceList[1 - sd][0].idx, flipMode);

                if (COL(pos0) > 3) {
                    flipMode = EgtbBoardCore::flip(flipMode, FlipMode::horizontal);
                    pos0 = EgtbBoardCore::flip(pos0, FlipMode::horizontal);
                    pos1 = EgtbBoardCore::flip(pos1, FlipMode::horizontal);
                }

                int kk = pos0 << 8 | pos1;
                int idx = bSearch(kk_2, EGTB_SIZE_KK2, kk);
                assert(idx >= 0 && idx < EGTB_SIZE_KK2);

                key += idx * mul;
                break;
            }

            case EGTB_IDX_KK_8:
            {
                int pos0 = EgtbBoardCore::flip(board.pieceList[sd][0].idx, flipMode);
                int pos1 = EgtbBoardCore::flip(board.pieceList[1 - sd][0].idx, flipMode);

                int flip = tb_flipMode[pos0];

                if (flip) {
                    auto flipMode2 = static_cast<FlipMode>(flip);
                    flipMode = EgtbBoardCore::flip(flipMode, flipMode2);
                    pos0 = EgtbBoardCore::flip(pos0, flipMode2);
                    pos1 = EgtbBoardCore::flip(pos1, flipMode2);
                }

                int kk = pos0 << 8 | pos1;
                int idx = bSearch(kk_8, EGTB_SIZE_KK8, kk);
                key += idx * mul;
                break;
            }

            case EGTB_IDX_K:
            {
                auto idx = EgtbBoardCore::flip(board.pieceList[sd][0].idx, flipMode);
                key += idx * mul;
                break;
            }

            case EGTB_IDX_Q:
            case EGTB_IDX_R:
            case EGTB_IDX_B:
            case EGTB_IDX_H:
            case EGTB_IDX_P:
            {
                PieceType type = static_cast<PieceType>(attr - EGTB_IDX_Q + 1);
                for(int t = 1; t < 16; t++) {
                    auto p = board.pieceList[sd][t];
                    if (!p.isEmpty() && p.type == type) {
                        auto pos = EgtbBoardCore::flip(p.idx, flipMode);
                        assert(pos >= 0 && pos < 64);

                        if (attr == EGTB_IDX_P) {
                            pos = EgtbKey::getKey_p(pos);
                        } else {
                            pos = EgtbKey::getKey_x(pos);
                        }
                        key += pos * mul;
                        break;
                    }
                }
                break;
            }

            case EGTB_IDX_QQ:
            case EGTB_IDX_RR:
            case EGTB_IDX_BB:
            case EGTB_IDX_HH:
            case EGTB_IDX_PP:
            {
                PieceType type = static_cast<PieceType>(attr - EGTB_IDX_QQ + 1);

                for(int t = 1; t < 16; t++) {
                    auto p0 = board.pieceList[sd][t];
                    if (!p0.isEmpty() && p0.type == type) {
                        auto idx0 = EgtbBoardCore::flip(p0.idx, flipMode);
                        for(t++; t < 16; t++) {
                            auto p1 = board.pieceList[sd ][t];
                            if (!p1.isEmpty() && p1.type == type) {
                                auto idx1 = EgtbBoardCore::flip(p1.idx, flipMode);

                                int subKey = type != PieceType::pawn ? EgtbKey::getKey_xx(idx0, idx1) : EgtbKey::getKey_pp(idx0, idx1);
                                key += subKey * mul;
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }

            case EGTB_IDX_QQQ:
            case EGTB_IDX_RRR:
            case EGTB_IDX_BBB:
            case EGTB_IDX_HHH:
            case EGTB_IDX_PPP:
            {
                PieceType type = static_cast<PieceType>(attr - EGTB_IDX_QQQ + 1);

                for(int t = 1; t < 16; t++) {
                    auto p0 = board.pieceList[sd][t];
                    if (!p0.isEmpty() && p0.type == type) {
                        auto idx0 = EgtbBoardCore::flip(p0.idx, flipMode);
                        for(t++; t < 16; t++) {
                            auto p1 = board.pieceList[sd][t];
                            if (!p1.isEmpty() && p1.type == type) {
                                auto idx1 = EgtbBoardCore::flip(p1.idx, flipMode);
                                for(t++; t < 16; t++) {
                                    auto p2 = board.pieceList[sd][t];
                                    if (!p2.isEmpty() && p2.type == type) {
                                        auto idx2 = EgtbBoardCore::flip(p2.idx, flipMode);

                                        int subKey = type != PieceType::pawn ? EgtbKey::getKey_xxx(idx0, idx1, idx2) : EgtbKey::getKey_ppp(idx0, idx1, idx2);
                                        key += subKey * mul;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }

            case EGTB_IDX_QQQQ:
            case EGTB_IDX_RRRR:
            case EGTB_IDX_BBBB:
            case EGTB_IDX_HHHH:
            case EGTB_IDX_PPPP:
            {
                PieceType type = static_cast<PieceType>(attr - EGTB_IDX_QQQQ + 1);

                for(int t = 1; t < 16; t++) {
                    auto p0 = board.pieceList[sd][t];
                    if (!p0.isEmpty() && p0.type == type) {
                        auto idx0 = EgtbBoardCore::flip(p0.idx, flipMode);
                        for(t++; t < 16; t++) {
                            auto p1 = board.pieceList[sd][t];
                            if (!p1.isEmpty() && p1.type == type) {
                                auto idx1 = EgtbBoardCore::flip(p1.idx, flipMode);
                                for(t++; t < 16; t++) {
                                    auto p2 = board.pieceList[sd][t];
                                    if (!p2.isEmpty() && p2.type == type) {
                                        auto idx2 = EgtbBoardCore::flip(p2.idx, flipMode);
                                        for(t++; t < 16; t++) {
                                            auto p3 = board.pieceList[sd][t];
                                            if (!p3.isEmpty() && p3.type == type) {
                                                auto idx3 = EgtbBoardCore::flip(p3.idx, flipMode);

                                                int subKey = type != PieceType::pawn ? EgtbKey::getKey_xxxx(idx0, idx1, idx2, idx3) : EgtbKey::getKey_pppp(idx0, idx1, idx2, idx3);
                                                key += subKey * mul;
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
    }

    assert(key >= 0);
    rec.key = key;
}

