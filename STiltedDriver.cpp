/*

Driver Code for the Tilted Shatranj Engine

TheTilted096, 2024. 

*/

#include "STiltedMoveGen.cpp"

int main(){
    bool toMove = 1;
    uint64_t* white = new uint64_t[7];
    uint64_t* black = new uint64_t[7];

    setStartPos(white, black, toMove);

    auto start = std::chrono::steady_clock::now();
    std::cout << perft(white, black, 6, 1, 0) << " positions\n";
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << duration.count() << "ms";


    delete[] white;
    delete[] black;

    return 0;
}