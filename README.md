NhatMinh Egtb - A Chess Endgame tablebase
==========

Overview
----------

NhatMinh Egtb is a chess endgame database (tablebase) released with 3-4-5 endgames and probing code. The probing code is  written in C++ (using standad C++11 library). Chess developer could use this to add freely and quickly tablebase probing to their chess engines.

Nothing revolutionary here. This project is simply a test base for my researches to test, verify, compare ideas and techniques which I have used for both two kinds of chess: (Western) chess and Xiangqi (Chinese chess). The Xiangqi egtb is called Felicity Egtb.

Since it is mainly for reasearch, both data (egtbs) and probing code are subjects to be frequently changed.


Technique details
--------------------

Some popular egtbs such as Nalimov, Gaviota and Sygyzy have been used to compare.

#### alpha version (0.1)

Brief: DTM; handle enpassant but not castle rights; keep only one-side data; 3.1 GB for all 3-4-5 endgames.

Index space (similar to Nalimov):
- Use triangle for pawnless endgames, half board for pawn endgames
- Pawns occupy ranks 2-7
- Use code to handle en passants but not data
- Kings never can be near each other, two kings combination take only 564 (for pawnless endgames) or 1806 (for pawn endgames)
- Two, three or four identical pieces (XX, XXX, XXXX) could be reduced further

Techniques:
- Metric: DTM (similar to Nalimov / Gaviota).
- Enpassants: yes
- Castle rights: no
- For all 3-4-5 men the index space is about 19.9 G. It may take about 40 GB to store (a bit larger than Gaviota - 38 GB)
- Compress algorithms: Lzma, level 9, 4kb blocks (similar to Gaviota)
- Permutation: test with all orders (similar to Sygyzy)
- One side data has been discurded to save space (similar to Sygyzy)


Download
-----------

(3+4 men come with probing source code already)

- [3 men](https://drive.google.com/open?id=1Y_LSOFiROXbU6oR_-VGIzf3a4eks8uNI)
- [4 men](https://drive.google.com/open?id=1zAEyRwxTf1AyFClAUhcGQ10KsV8rajGN)
- [5 men pawnless](https://drive.google.com/open?id=1k3nW37k6vuJK8TkRt2rfAqzapnLFZ_Dw)
- [5 men pawn (4+1p)](https://drive.google.com/open?id=1mnMX9czIovupSX01APZppdxr-AqQVyew)
- [5 men pawn with queen (3+2qp)](https://drive.google.com/open?id=1ESvyT-O9pxWNOSGpdu101iNXjDnmtyFA)
- [5 men pawn without queen (3+2p)](https://drive.google.com/open?id=1df5l_LFqRc-DSsouIGa-JfzQ-aPhf3UO)

Total size: 3.14 GB


Working
---------

### Failed tries
- Compress libraries / sdks: zlib, zstandard, brotli (compress ratios are wose than lzma)
- Two King combinations (current applying) look like not a gain for compressed data.

### Working
- Reduce sizes
- 6 men

Demo
-------

The source code is realeased with a demo in the file main.cpp (remove this file when adding source to your project). You may compile and run it.

#### MacOS with XCode
Click to open FelicityEgtb.xcodeproj with XCode and run it

#### Linux / MacOS with gcc, g++

    bash comp.sh

#### Windows with VisualStudio
Click to open VisualStudio.sln with VisualStudio  (2017) and run it


Using
----------

To declare:

    #include "Egtb.h"
    ...
    egtb::EgtbDb egtbDb;

To init data you must give the main folder of the tablebase. The library will auto scan that folder as well as any sub folders to load all data it has known:

    const char* egtbDataFolder = "c:\\myfolder\\egtb";
    egtbDb.preload(egtbDataFolder, egtb::EgtbMemMode::all, egtb::EgtbLoadMode::loadnow);

You may check if it could load some tablebases and print out an error message:

    if (egtbDb.getSize() == 0) {
        std::cerr << "Error: could not load any data" << std::endl;
    }

With memory mode egtb::EgtbMemMode::all as above example, all data will be loaded, auto decompressed into memory (RAM) and the library won't access external storage anymore. With mode egtb::EgtbMemMode::small, the library will load only files' headers into memory and alloc some buffers in the memory (total about few MB). If the data in those buffers are out of range (missed the caches), The library will access external storage to read data in block, decompressed and return results when probing.

Now you may query scores (distance to mate) for any position. Your input could be FEN strings or vectors of pieces which each piece has type, side and location:

    std::vector<egtb::Piece> pieces;
    pieces.push_back(egtb::Piece(egtb::PieceType::king, egtb::Side::white, egtb::Squares::B3));
    pieces.push_back(egtb::Piece(egtb::PieceType::rook, egtb::Side::white, egtb::Squares::A5));
    pieces.push_back(egtb::Piece(egtb::PieceType::king, egtb::Side::black, egtb::Squares::G8));
    pieces.push_back(egtb::Piece(egtb::PieceType::queen, egtb::Side::black, egtb::Squares::H1));

    auto score = egtbDb.getScore(pieces);
    std::cout << "Queried, score: " << score << std::endl;


Compile
----------

If you use other C++ IDE such as Visual Studio, you need to creat a new project and add all our code with prefix Egtb and all file in sub folder lzma, set compile flags to C++11, then those IDEs can compile those source code automatically.

In case you want to compile those code manually with gcc, g++, you can use gcc to compile all C files, g++ for cpp files, then use g++ to link into executive file:

    gcc -std=c99 -c ./lzma/*.c
    g++ -std=c++11 -c *.cpp -O3 -DNDEBUG
    g++ -o yourenginename *.o


History
----------

13 Jan 2018: version alpha 0.01. Data size 3.1 GB


Terms of use
--------------

The files in folder lzma are copyrighted by 7-zip.org and were released under public domain.
Our code and data (egtb files) are released under the liberal [MIT license](http://en.wikipedia.org/wiki/MIT_License), so basically you can use it with almost no restrictions.


Credits
--------

NhatMinh Egtb was written by Nguyen Hong Pham (axchess at yahoo dot com)


