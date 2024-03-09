/*
This is take 2 of the Tilted Shatranj Engine,
since git decided it was a great idea to kill everybody. 

Anyways, this is a library to do movegen. 

TheTilted096, 3-8-2024. 
*/

#include <iostream>
#include <bit>
#include <cstdint>

#define RANK0 255ULL //8th rank
#define FILE0 72340172838076673ULL //A File

/*
The plan:

Each side is still represented as 7 bitboards. 

Order of Pieces; King, Rook, Knight, Ferz, Alfil, Pawn, ALL
*/

void printAsBitboard(uint64_t board){
    for (int i = 0; i < 8; i++){
        std::cout << (board & 255ULL) << '\n';
        board >>= 8;
    }
}



uint64_t emptyBoardMoves(int square, int pieceType){
    return 0;
}