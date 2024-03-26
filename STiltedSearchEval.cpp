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

    fullMoveGen(ply);

    for (int i = 0; i < moves[ply][0]; i++){ //for each move
        makeMove(moves[ply][i + 1], 1, 1); //make the move.

        endHandle();

        if (isChecked() or kingBare()){ //if is checked
            //std::cout << "\nMove Rejected: " << moveToAlgebraic(moves[ply][i + 1]) << '\n';
            makeMove(moves[ply][i + 1], 0, 1); //unmake the move
            continue;
        }

        score = -alphabeta(-beta, -alpha, depth - 1, ply + 1); //do for opp
        if (score >= beta){ //if opp makes a bad move (they would not do this)
            makeMove(moves[ply][i + 1], 0, 1); //unmake the move.
            return beta;   //  fail hard beta-cutoff
        }
        if (score > alpha){ //yay best move
            if (ply == 0){ //if this is the root call, save the best move
                bestMove = moves[ply][i + 1];
            }
            alpha = score; // alpha acts like max in MiniMax  
        }      
        makeMove(moves[ply][i + 1], 0, 1); //unmake the move.
    }

    if (score == -29000){
        score += ply;
        return score;
    }


    return alpha;
}

void moveTimer(int wait){
    timeExpired = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    timeExpired = true;
}

int iterativeDeepening(int alpha, int beta, int thinkTime, int mdepth){
    std::thread myTimer;
    if (~thinkTime){
        myTimer = std::thread(moveTimer, thinkTime);
    }

    uint32_t cbMove;
    int cbEval;
    uint64_t prevNodes;

    evaluate(); //set wtotal and btotal before search

    auto start = std::chrono::steady_clock::now();
    try {
        for (int i = 0; i < mdepth; i++){
            prevNodes = nodes;
            cbEval = alphabeta(alpha, beta, i, 0);
            cbMove = bestMove;
            std::cout << "info depth " << i << " nodes " << nodes - prevNodes << " score cp " << cbEval << '\n';
        }
    } catch (const char* e){
        timeExpired = false;
        std::cout << e;
        boardEval = cbEval;
        bestMove = cbMove;
    }
    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    myTimer.detach();

    std::cout << "info nodes " << nodes << " nps ";
    if (dur == 0){
        std::cout << "0\n";
    } else {
        std::cout  << (int) ((float) nodes * 1000 / dur);
    }
    std::cout << "\nbestmove " << moveToAlgebraic(bestMove) << '\n';

    return boardEval;
}



