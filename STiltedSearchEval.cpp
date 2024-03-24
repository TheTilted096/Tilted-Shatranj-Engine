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

/*
Move Representation:

0000 0000 0000 0000 0000 000000 000000

0-5: start square
6-11: end square

12: Capture
13-15: Captured Type
16-18: Piece Type Moved
19: Promotion
20-22: Piece End Type
23: Color

*/

int evaluate(){
    wtotal = 0, btotal = 0;
    int values[6] = {0, 650, 400, 200, 150, 100};

    uint8_t *wlist;
    uint8_t *blist;

    for (int i = 0; i < 6; i++){
        wlist = bitboardToList(white[i]);
        blist = bitboardToList(black[i]);

        wtotal += (__builtin_popcountll(white[i]) * values[i]);
        btotal += (__builtin_popcountll(black[i]) * values[i]);

        for (int j = 0; j < wlist[0]; j++){
            wtotal += mps[i][wlist[j + 1]]; // add psqt bonus at piece's square
        }
        for (int j = 0; j < blist[0]; j++){
            btotal += mps[i][56 ^ blist[j + 1]];
        }
        delete[] wlist;
        delete[] blist;
    }

    if (toMove){ // if white to move
        return (wtotal - btotal);
    } else {
        return (btotal - wtotal);
    }

    return 0;
}

bool kingBare(){ //returns true if 'toMove' king is bare, false otherwise, and false when both bare.
    bool b = (black[6] == black[0]);
    bool w = (white[6] == white[0]);

    return ((toMove and !w and b) or (!toMove and w and !b)); // (black tM and white not bare and black bare) OR (white tM and white bare and black not bare);
}

int alphabeta(int alpha, int beta, int depth, int ply){
    int score = -29000;

    if (depth == 0){
        if (toMove){ // if white to move
            return (wtotal - btotal);
        } else {
            return (btotal - wtotal);
        }
    }

    uint32_t* moves = fullMoveGen();

    for (int i = 0; i < moves[0]; i++){ //for each move
        makeMove(moves[i + 1], 1, 1); //make the move.

        if (isChecked() or kingBare()){ //if is checked
            //std::cout << "\nMove Rejected: " << moveToAlgebraic(moves[i + 1]) << '\n';
            makeMove(moves[i + 1], 0, 1); //unmake the move
            continue;
        }

        score = -alphabeta(-beta, -alpha, depth - 1, ply + 1); //do for opp
        if (score >= beta){ //if opp makes a bad move (they would not do this)
            makeMove(moves[i + 1], 0, 1); //unmake the move.

            delete[] moves;
            return beta;   //  fail hard beta-cutoff
        }
        if (score > alpha){ //yay best move
            if (ply == 0){ //if this is the root call, save the best move
                bestMove = moves[i + 1];
            }
            alpha = score; // alpha acts like max in MiniMax  
        }      
        makeMove(moves[i + 1], 0, 1); //unmake the move.
    }

    if (score == -29000){
        score += ply;
        delete[] moves;
        return score;
    }

    delete[] moves;

    return alpha;
}
