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


    /*
    //00000000 1 010 0 010 000 0 101000 111001
    uint32_t b1a3 = 0b00000000101000100000101000111001;
    makeMove(b1a3, white, black, 1);
    //printSidesBitboard(white);

    //00000000 0 010 0 010 000 0 010000 000001
    uint32_t b8a6 = 0b00000000001000100000010000000001;
    makeMove(b8a6, white, black, 1);
    //printSidesBitboard(black);
   
    //00000000 1 010 0 010 000 0 011001 101000
    uint32_t a3b5 = 0b00000000101000100000011001101000;
    makeMove(a3b5, white, black, 1);

    /*
    //00000000 0 101 0 101 000 0 010011 001011
    uint32_t d7d6 = 0b00000000010101010000010011001011;
    makeMove(d7d6, white, black, 1);
    */
    
    std::cout << perft(white, black, 6, 1, 0);




    delete[] white;
    delete[] black;

    return 0;
}