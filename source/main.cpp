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

#include <iostream>

/*
 * Need to add only one .h file
 */
#include "Egtb.h"

/*
 * The EGT should declare only one since it may take time to load data and some memory
 */
egtb::EgtbDb egtbDb;

// Utility function
std::string explainScore(int score);

int main() {
    std::cout << "Welcome to NhatMinh Chess Endgame databases - version: " << egtb::getVersion() << std::endl;

    /*
     * Allow Egtb to print out more information
     */
    egtb::egtbVerbose = true;

    /*
     * Preload endgames into memory
     * Depend on memory mode, it could load partly or whole data into memory
     * If this process of loading is slow (when you load all into memory for huge endgames),
     * you may use a background thread to load it and use when data is ready
     */
    // Note: this project has been attached already endgames 3-4 men. For 5 men you need to download separately
    const char* egtbDataFolder = "./egtb";

    // You may add folders one by one
    egtbDb.addFolders(egtbDataFolder);

    // Data can be loaded all into memory or tiny or smart (let programm decide between all-tiny)
    egtb::EgtbMemMode egtbMemMode = egtb::EgtbMemMode::all;
    // Data can be loaded right now or don't load anything until the first request
    egtb::EgtbLoadMode loadMode = egtb::EgtbLoadMode::onrequest;

    egtbDb.preload(egtbMemMode, loadMode);

    // The numbers of endgames should not be zero
    if (egtbDb.getSize() == 0) {
        std::cerr << "Error: Egtb could not load any endgames from folder " << egtbDataFolder << ". 3 + 4 egtb should have totally 35 endgames. Please check!" << std::endl;
        return -1;
    }

    std::cout << "Egtb database size: " << egtbDb.getSize() << std::endl << std::endl;

    /*
     * Query scores
     * To enter a chess board to query, you may use a fen strings or an array of pieces, each piece has piece type, side and position
     * You may put input (fen string or vector of pieces) directly to egtb or via internal board of egtb
     *
     * WARNING: the chess board must be valid, otherwise the score may have no meaning (just a random number)
     */

    // Query with internal board of egtb
    // The advantages of using that board you may quickly show the board or check the valid
    egtb::EgtbBoard board;

    board.setFen(""); // default starting board
    board.show();

    auto score = egtbDb.getScore(board);
    std::cout << "Query the starting board, score: " << score << ", explaination: " << explainScore(score) << std::endl << std::endl;

    board.setFen("K2k4/2p5/8/8/8/8/8/8 w - - 0 1");
    board.show();

    score = egtbDb.getScore(board);
    std::cout << "Query with a fen string, score: " << score << ", explaination: " << explainScore(score) << std::endl << std::endl;

    // Use a vector of pieces
    /*
     Squares is defined in Egtb.h as below:
     enum Squares {
     A8, B8, C8, D8, E8, F8, G8, H8,
     A7, B7, C7, D7, E7, F7, G7, H7,
     A6, B6, C6, D6, E6, F6, G6, H6,
     A5, B5, C5, D5, E5, F5, G5, H5,
     A4, B4, C4, D4, E4, F4, G4, H4,
     A3, B3, C3, D3, E3, F3, G3, H3,
     A2, B2, C2, D2, E2, F2, G2, H2,
     A1, B1, C1, D1, E1, F1, G1, H1
     };
     */

    std::vector<egtb::Piece> pieces;
    pieces.push_back(egtb::Piece(egtb::PieceType::king, egtb::Side::white, egtb::Squares::B3));
    pieces.push_back(egtb::Piece(egtb::PieceType::rook, egtb::Side::white, egtb::Squares::A5));

    pieces.push_back(egtb::Piece(egtb::PieceType::king, egtb::Side::black, egtb::Squares::G8));
    pieces.push_back(egtb::Piece(egtb::PieceType::queen, egtb::Side::black, egtb::Squares::H1));

    if (board.setup(pieces, egtb::Side::white) && board.isValid()) { // vector of pieces and side to move
        board.show();
        auto score = egtbDb.getScore(board);
        std::cout << "Query with a vector of pieces, score: " << score << ", explaination: " << explainScore(score) << std::endl << std::endl;
    } else {
        std::cerr << "Error on board setup" << std::endl;
    }

    // Not use internal board:
    score = egtbDb.getScore(pieces, egtb::Side::black);
    std::cout << "Query directly (not using internal board) with a vector of pieces, score: " << score << ", explaination: " << explainScore(score) << std::endl << std::endl;

    /*
     * Probe a position via a vector of pieces
     * Different from getScore, probe will return a list of moves which lead to the mate
     */
    egtb::MoveList moveList;
    score = egtbDb.probe(board, moveList);
    std::cout << "Probe directly with a vector of pieces, score: " << score << ", explaination: " << explainScore(score) << std::endl;
    std::cout << "moves to mate: " << moveList.toString() << std::endl << std::endl;


    return 0;
}

std::string explainScore(int score) {
    std::string str = "";

    switch (score) {
        case EGTB_SCORE_DRAW:
            str = "draw";
            break;

        case EGTB_SCORE_MISSING:
            str = "missing (board is incorrect or missing some endgame databases)";
            break;
        case EGTB_SCORE_MATE:
            str = "mate";
            break;

        case EGTB_SCORE_WINNING:
            str = "winning";
            break;

        case EGTB_SCORE_ILLEGAL:
            str = "illegal";
            break;

        case EGTB_SCORE_UNKNOWN:
            str = "unknown";
            break;

        default: {
            auto mateInPly = EGTB_SCORE_MATE - abs(score);
            int mateIn = (mateInPly + 1) / 2; // devide 2 for full moves
            if (score < 0) mateIn = -mateIn;

            std::ostringstream stringStream;
            stringStream << "mate in " << mateIn << " (" << mateInPly << " " << (mateInPly <= 1 ? "ply" : "plies") << ")";
            str = stringStream.str();
            break;
        }
    }
    return str;
}

