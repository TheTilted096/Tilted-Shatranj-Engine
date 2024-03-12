/*

Library for Tilted Shatranj Engine's Search and Eval. 

Nathaniel Potter, 3-11-2024

*/

#include "STiltedMoveGen.cpp"

/*
#define ROOK_VALUE 500
#define KNIGHT_VALUE 300
#define FERZ_VALUE 200
#define ALFIL_VALUE 150
#define PAWN_VALUE 100
*/

int evaluate(uint64_t* white, uint64_t* black, bool toMove){
    int wmtotal = 0;
    int bmtotal = 0;

    int centiValues[5] = {500, 300, 200, 150, 100};

    for (int i = 1; i < 6; i++){
        wmtotal += ((__builtin_popcountll(white[i])) * centiValues[i - 1]);
        bmtotal += ((__builtin_popcountll(black[i])) * centiValues[i - 1]);
    }
    int val = wmtotal - bmtotal;
    int noise = rand() % 32;
    
    if (toMove){
        return val + noise;
    } else {
        return -val-noise;
    }

    return 0;
}

std::string search(uint64_t*& white, uint64_t*& black, bool toMove, int& ev){
    uint32_t* moves = fullMoveGen(white, black, toMove);
    int bestScore = -INT_MAX; //initialize to negative INF
    int currScore;
    int bestIndex;

    for (int i = 0; i < moves[0]; i++){ //for each move
        makeMove(moves[i + 1], white, black, 1); //make the move. 
        if (isChecked(toMove, white, black)){ //if is checked
            makeMove(moves[i + 1], white, black, 0); //unmake the move
            continue;
        }
        currScore = evaluate(white, black, toMove); //evaluate the position
        if (currScore > bestScore){ //if it is good, set the best score
            bestScore = currScore;
            bestIndex = i + 1; //keep track of the best move thus far for return.
            
        }
        makeMove(moves[i + 1], white, black, 0); //unmake the move. 
    }

    ev = bestScore;

    return moveToAlgebraic(moves[bestIndex]);
}
