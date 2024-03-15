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

bool kingBare(uint64_t* white, uint64_t* black, bool toMove){ //returns true if 'toMove' king is bare, false otherwise, and false when both bare.
    bool b = (black[6] == black[0]);
    bool w = (white[6] == white[0]);

    return ((!toMove and !w and b) or (toMove and w and !b)); // (black tM and white not bare and black bare) OR (white tM and white bare and black not bare);
}

int alphabeta(uint64_t*& white, uint64_t*& black, bool toMove, int alpha, int beta, int depth, uint32_t& bestMove, int ply, uint64_t*** tables){
    int score = -29999;

    if (depth == 0){
        return evaluate(white, black, toMove);
    }

    uint32_t* moves = fullMoveGen(white, black, toMove, tables);

    for (int i = 0; i < moves[0]; i++){ //for each move
        makeMove(moves[i + 1], white, black, 1); //make the move. 
        if (isChecked(toMove, white, black, tables) or kingBare(white, black, toMove)){ //if is checked
            makeMove(moves[i + 1], white, black, 0); //unmake the move
            continue;
        }
        score = -alphabeta(white, black, !toMove, -beta, -alpha, depth - 1, bestMove, ply + 1, tables); //do for opp
        if (score >= beta){ //if opp makes a bad move (they would not do this)
            makeMove(moves[i + 1], white, black, 0); //unmake the move.
            delete[] moves;
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

    if (score == -29999){
        delete[] moves;
        return score;
    }

    delete[] moves;

    return alpha;
}
