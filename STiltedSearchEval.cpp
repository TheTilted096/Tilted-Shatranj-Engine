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

int alphabeta(uint64_t*& white, uint64_t*& black, bool toMove, int alpha, int beta, int depth, uint32_t& bestMove, int ply){
    uint32_t* moves = fullMoveGen(white, black, toMove);

    int score = -29000;

    if (depth == 0){
        return evaluate(white, black, toMove);
    }

    for (int i = 0; i < moves[0]; i++){ //for each move
        makeMove(moves[i + 1], white, black, 1); //make the move. 
        if (isChecked(toMove, white, black)){ //if is checked
            makeMove(moves[i + 1], white, black, 0); //unmake the move
            continue;
        }
        score = -alphabeta(white, black, !toMove, -beta, -alpha, depth - 1, bestMove, ply + 1); //do for opp
        if (score >= beta){ //if opp makes a bad move (they would not do this)
            makeMove(moves[i + 1], white, black, 0); //unmake the move.
            return beta;   //  fail hard beta-cutoff
        }
        if (score > alpha){ //yay best move
            if (ply == 0){ //if this is the root call, save the best move
                bestMove = moves[i + 1];
            }
            alpha = score; // alpha acts like max in MiniMax  
        }      
        makeMove(moves[i + 1], white, black, 0); //unmake the move. 
    }

    if (score == -29000){
        return score;
    }

    return alpha;
}
