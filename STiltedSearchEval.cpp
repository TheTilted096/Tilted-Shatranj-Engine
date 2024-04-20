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

class TTentry{
    public: 
        int entryScore;
        uint64_t hash;
        int nodetype;
        int depthAt;

        uint32_t entryMove;

        TTentry(){
            entryScore = -29501;
            hash = 0;
            nodetype = -1;
            depthAt = -1;
            entryMove = 0;
        }

        void update(int sc, int nt, int d, uint32_t dm){
            if ((d >= depthAt) or (zHistory[totalHalfMoves] != hash)){ 
                //if depth is more than our depth or is a new position
                entryScore = sc;
                nodetype = nt;
                depthAt = d;
                hash = zHistory[totalHalfMoves];
                entryMove = dm;
            }
        }

        void print(){
            std::cout << "Index: " << (hash & 0xFFFFF) << "     Score: " << entryScore
                << "\tdepthAt: " << depthAt << "     nodeType: " << nodetype << "     debugMove: " 
                    << moveToAlgebraic(entryMove) << '\n';
        }
};

TTentry ttable[1048576];

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

int quiesce(int alpha, int beta, int lply){
    int failSoft = toMove ? (wtotal - btotal) : (btotal - wtotal); //quickly evaluate the position
    int score = -29000; //initialize the score 
    if (failSoft >= beta){
        return beta;
    }
    if (alpha < failSoft){
        alpha = failSoft;
    }

    fullMoveGen(64 + lply, 1); //generate moves and write them in a separate part of the array
    
    for (int aa = 2; aa < moves[64 + lply][0] + 1; aa++){
        //swap if victim is more valuable then if agro is less valuable
        //swap bb-1 and bb if ''
        //then say bb - 1 > bb if index(bb - 1) > bb
        //index = victim * 10 - agro
        for (int bb = aa; bb > 1 and 
            (10 * ((moves[64 + lply][bb - 1] >> 13) & 7) - ((moves[64 + lply][bb - 1] >> 16) & 7)) 
            > (10 * ((moves[64 + lply][bb] >> 13) & 7) - ((moves[64 + lply][bb] >> 16) & 7))
            ; bb--){
            std::swap(moves[64 + lply][bb - 1], moves[64 + lply][bb]);
        }
    }

    for (int i = 0; i < moves[64 + lply][0]; i++){ //for each move
        makeMove(moves[64 + lply][i + 1], 1, 1); //make the move.
        endHandle(); //if there are time/node constraints, handle them
        if (isChecked() or kingBare()){ //if is checked
            //std::cout << "\nMove Rejected: " << moveToAlgebraic(moves[ply][i + 1]) << '\n';
            makeMove(moves[64 + lply][i + 1], 0, 1); //unmake the move
            continue;
        }
        score = -quiesce(-beta, -alpha, lply + 1);
        makeMove(moves[64 + lply][i + 1], 0, 1); //take back the move
        
        if (score >= beta){
            return beta;
        }
        if (score > alpha){ //yay best move
            alpha = score; // alpha acts like max in MiniMax  
        }   
    }
    return alpha;
}

int moveStrength(uint32_t& move){
    //the stronger the move, the lower
    int strength = ((move >> 16) & 7) * (-1); //less valuable = better (-a)
    if ((move >> 12) & 1){ //if capturing
        strength += (10 * ((move >> 13) & 7)); //10v
    } else {
        strength += 100;
    }

    return strength;    
}

int alphabeta(int alpha, int beta, int depth, int ply, bool nmp){
    int score = -29000;

    int index = totalHalfMoves; //start iterator
    int reps = 0; //keep track of repetitions
    while (currentHalfMoves[index] != 0){ //search until you hit a reset
        if (zHistory[totalHalfMoves] == zHistory[index]){ //compare current position 
            reps++; //if equal, you found repetition
        }
        index -= 2;
    }
    if (reps > 2){ //if more than 2 repetitions (3), its a draw; unmake and return 0
        return 0;
    }

    if (depth == 0){
        return quiesce(alpha, beta, 0);
    }

    //if no index collision and TT depth > depth and this is not the root call (might change later)
    int ttindex = zHistory[totalHalfMoves] & 0xFFFFF;
    if ((zHistory[totalHalfMoves] == ttable[ttindex].hash) and (ttable[ttindex].depthAt >= depth) 
            and (ply > 0) and (reps == 1)){
        score = ttable[ttindex].entryScore;

        if (ttable[ttindex].nodetype == 1){ //PV
            return score;
        }
        if ((ttable[ttindex].nodetype == 2) and (score >= beta)){ //cut
            return score;
        }
        if ((ttable[ttindex].nodetype == 3) and (score <= alpha)){ //all
            return score;
        }
    }

    nmp = ((wtotal > 1000) and (btotal > 1000)) and !nmp;
    //nmp enabled if both sides have material and there was no nmp before
    //nmp disabled if both sides dont have material or there was nmp before

    if (nmp and (ply != 0) and (depth > 1)) {
        makeMove(0, 1, 1);
        if (!isChecked()) {
            score = -alphabeta(-beta, -alpha, depth - 2, ply + 1, nmp);
        }
        makeMove(0, 0, 1);
        if (score >= beta) {
            return beta;
        }
    }

    uint32_t localBestMove = 0;
    bool isAllNode = true; 

    fullMoveGen(ply, 0);

    bool ttMoveFound = false;

    for (int ll = 0; ll < moves[ply][0]; ll++){
        if (moves[ply][ll + 1] == ttable[ttindex].entryMove){
            uint32_t tempMove = moves[ply][1];
            moves[ply][1] = ttable[ttindex].entryMove;
            moves[ply][ll + 1] = tempMove;
            ttMoveFound = true;
            break;
        }
    }

    for (int aa = 2 + ttMoveFound; aa < moves[ply][0] + 1; aa++){
        for (int bb = aa; (moveStrength(moves[ply][bb - 1]) > moveStrength(moves[ply][bb])) and (bb > 1 + ttMoveFound); bb--){
            std::swap(moves[ply][bb], moves[ply][bb - 1]);
        }
    }

    for (int i = 0; i < moves[ply][0]; i++){ //for each move
        makeMove(moves[ply][i + 1], 1, 1); //make the move.

        endHandle();

        if (isChecked() or kingBare()){ //if is checked
            makeMove(moves[ply][i + 1], 0, 1); //unmake the move
            continue;
        }

        score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp); //do for opp

        makeMove(moves[ply][i + 1], 0, 1); //unmake the move.

        if (score >= beta){ //if opp makes a bad move (they would not do this)
            //beta cutoff; cut-nodes (2) 
            ttable[ttindex].update(score, 2, depth, moves[ply][i + 1]);
            return beta;   //  fail hard beta-cutoff
        }
        if (score > alpha){ //yay best move
            if (ply == 0){ //if this is the root call, save the best move
                bestMove = moves[ply][i + 1];
            }

            //PV-nodes (1)
            localBestMove = moves[ply][i + 1];
            isAllNode = false;

            alpha = score; // alpha acts like max in MiniMax  
        }
    }

    if (score == -29000){
        score += ply;
        return score;
    }
    if (isAllNode){ 
        ttable[ttindex].update(alpha, 3, depth, 0); 
    } else {
        ttable[ttindex].update(alpha, 1, depth, localBestMove);
    }
    return alpha;
}

void moveTimer(int wait){
    timeExpired = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    timeExpired = true;
}

int iterativeDeepening(int alpha, int beta, uint32_t thinkTime, int mdepth){
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
        for (int i = 0; i < mdepth + 1; i++){
            prevNodes = nodes;
            cbEval = alphabeta(alpha, beta, i, 0, false); //ensure nmp = 1 when ply = 0
            cbMove = bestMove;
            std::cout << "info depth " << i << " nodes " << nodes - prevNodes << " score cp " << cbEval << '\n';
        }
    } catch (const char* e){
        timeExpired = false;
        //std::cout << e;
        boardEval = cbEval;
        bestMove = cbMove;
    }
    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (~thinkTime){
        myTimer.detach();
    }
        
    std::cout << "info nodes " << nodes << " nps ";
    if (dur == 0){
        std::cout << "0\n";
    } else {
        std::cout  << (int) ((float) nodes * 1000 / dur);
    }
    std::cout << "\nbestmove " << moveToAlgebraic(bestMove) << '\n';

    return boardEval;
}



