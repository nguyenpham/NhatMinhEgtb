mkdir exect
cd exect
rm *
gcc -std=c99 -c ../source/lzma/*.c
g++ -std=c++11 -c ../source/*.cpp -O2 -DNDEBUG
g++ -o nmegtbdemo *.o
rm *.o
cd ..
./exect/nmegtbdemo
