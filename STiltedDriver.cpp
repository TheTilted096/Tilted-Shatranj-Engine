/*

Driver Code for the Tilted Shatranj Engine

TheTilted096, 2024. 

*/

#include "STiltedMoveGen.cpp"

int main(){
    uint64_t* white = new uint64_t[7];
    uint64_t* black = new uint64_t[7];

    black[0] = 8ULL;
    black[1] = 129ULL;
    black[2] = 66ULL;
    black[3] = 16ULL;
    black[4] = 36ULL;
    black[5] = RANK0 << 8;
    black[6] = RANK0 | (RANK0 << 8);

    for (int i = 0; i < 5; i++){
        white[i] = black[i] << 56;
    }

    white[5] = (RANK0 << 48);
    white[6] = (RANK0 << 48) | (RANK0 << 56);

    //printSidesBitboard(white);

    for (int i = 0; i < 7; i++){
        std::cout << perft(white, black, i, 1, 0) << '\n';
    }




    delete[] white;
    delete[] black;

    return 0;
}